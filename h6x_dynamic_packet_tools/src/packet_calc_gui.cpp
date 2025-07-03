/*
 * Copyright (c) 2025 HarvestX Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <vector>
#include <cstring>
#include "ui_packet_calc_gui.h"

#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_definitions_base.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_big_endian.h"
#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_crc8.h"
#include "h6x_dynamic_serial_port_handler/serial_port_handler.hpp"

class PacketCalcGUI : public QMainWindow
{
  Q_OBJECT

public:
  PacketCalcGUI(
    const QString & commandLinePort = QString(), const QString & role = "host",
    QWidget * parent = nullptr);
  ~PacketCalcGUI();

private slots:
  void connectToDevice();
  void disconnectFromDevice();
  void calculatePacket();
  void sendCustomPacket();
  void onPacketDataChanged();
  void onDataReceived(const Packet & packet);
  void onClientReceiveTimer();

private:
  void setupUI();
  void updateConnectionStatus(bool connected);
  void logMessage(const QString & message);
  void populateSerialPorts();
  std::vector<int16_t> parsePacketData(const QString & text);
  uint8_t calculateCRC(uint8_t client_id, uint8_t mode, const std::vector<uint8_t> & data);
  void updatePacketCalculation();
  void updateResponseDisplay(const Packet & response, bool success);
  void setSendingState(bool sending);

  // UI Components
  Ui::PacketCalcGUI * ui;

  // Communication
  h6x_dynamic_serial_port_handler::SerialPortHandler * interface;
  bool isConnected;
  bool isSending;
  QString commandLinePort;
  QString deviceRole;
  QTimer * clientReceiveTimer;
};

PacketCalcGUI::PacketCalcGUI(
  const QString & commandLinePort, const QString & role,
  QWidget * parent)
: QMainWindow(parent), ui(new Ui::PacketCalcGUI), interface(nullptr), isConnected(false), isSending(
    false), commandLinePort(commandLinePort), deviceRole(role)
{
  setupUI();

  interface = new h6x_dynamic_serial_port_handler::SerialPortHandler();


  // Setup client receive timer with longer interval to reduce load
  clientReceiveTimer = new QTimer(this);
  connect(clientReceiveTimer, &QTimer::timeout, this, &PacketCalcGUI::onClientReceiveTimer);
  clientReceiveTimer->setInterval(200);

  populateSerialPorts();
  updatePacketCalculation();
}

PacketCalcGUI::~PacketCalcGUI()
{
  // Stop timers first
  if (clientReceiveTimer && clientReceiveTimer->isActive()) {
    clientReceiveTimer->stop();
  }


  if (interface) {
    delete interface;
  }
  delete ui;
}

void PacketCalcGUI::setupUI()
{
  ui->setupUi(this);

  QString title = this->windowTitle();
  if (deviceRole == "host") {
    setWindowTitle(title + " (HOST)");
  } else if (deviceRole == "client") {
    setWindowTitle(title + " (CLIENT)");
    ui->packetDataTextEdit->setEnabled(false);
    ui->calculateButton->setEnabled(false);
    ui->sendPacketButton->setEnabled(false);
    ui->CommandSpinBox->setEnabled(false);
    ui->clientIdSpinBox->setEnabled(false);
  }

  // Connect signals
  connect(ui->connectButton, &QPushButton::clicked, this, &PacketCalcGUI::connectToDevice);
  connect(ui->disconnectButton, &QPushButton::clicked, this, &PacketCalcGUI::disconnectFromDevice);
  connect(ui->calculateButton, &QPushButton::clicked, this, &PacketCalcGUI::calculatePacket);
  connect(ui->sendPacketButton, &QPushButton::clicked, this, &PacketCalcGUI::sendCustomPacket);
  connect(
    ui->packetDataTextEdit, &QTextEdit::textChanged, this,
    &PacketCalcGUI::onPacketDataChanged);
  connect(
    ui->clientIdSpinBox, QOverload<int>::of(
      &QSpinBox::valueChanged), this, &PacketCalcGUI::onPacketDataChanged);
  connect(
    ui->CommandSpinBox, QOverload<int>::of(
      &QSpinBox::valueChanged), this, &PacketCalcGUI::onPacketDataChanged);
}

void PacketCalcGUI::populateSerialPorts()
{
  ui->portComboBox->clear();

  if (!commandLinePort.isEmpty()) {
    ui->portComboBox->addItem(commandLinePort);
    ui->portComboBox->setCurrentIndex(0);
    ui->portComboBox->setEnabled(false);
    return;
  }

  QDir dir("/dev");
  QStringList filters;
  filters << "ttyACM*";
  filters << "ttyUSB*";
  QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
  for (const QFileInfo & file : files) {
    ui->portComboBox->addItem(file.filePath());
  }
  if (ui->portComboBox->count() > 0) {
    ui->portComboBox->setCurrentIndex(0);
  } else {
    ui->portComboBox->addItem("No serial ports found?");
    ui->portComboBox->setEnabled(false);
  }
}

void PacketCalcGUI::connectToDevice()
{
  QString port = ui->portComboBox->currentText();
  int baudrate = ui->baudrateSpinBox->value();

  if (interface->init_serial(port.toStdString(), baudrate)) {
    interface->set_data_callback(
      [this](const Packet & recv_pkt) {
        this->onDataReceived(recv_pkt);
      });

    isConnected = true;
    updateConnectionStatus(true);
    logMessage("Connected to device on " + port);

    // Start client receive timer if in client mode
    if (deviceRole == "client") {
      clientReceiveTimer->start();
      logMessage("Started client receive timer (200ms interval)");
    }
  } else {
    QMessageBox::warning(
      this, "Connection Error",
      "Failed to connect to device on " + port);
    logMessage("Failed to connect to device on " + port);
  }
}

void PacketCalcGUI::disconnectFromDevice()
{
  if (isConnected) {
    isConnected = false;
    updateConnectionStatus(false);
    logMessage("Disconnected from device");

    // Stop client receive timer
    if (clientReceiveTimer->isActive()) {
      clientReceiveTimer->stop();
      logMessage("Stopped client receive timer");
    }
  }
}

void PacketCalcGUI::updateConnectionStatus(bool connected)
{
  ui->connectButton->setEnabled(!connected);
  ui->disconnectButton->setEnabled(connected);

  if (connected) {
    ui->connectionStatusLabel->setText("Connected");
    ui->connectionStatusLabel->setStyleSheet("color: green;");
  } else {
    ui->connectionStatusLabel->setText("Disconnected");
    ui->connectionStatusLabel->setStyleSheet("color: red;");
  }

  ui->packetCalculatorGroup->setEnabled(connected && !isSending);
}

void PacketCalcGUI::onDataReceived(const Packet & packet)
{
  QString logMsg = QString("Data received - Command: 0x%1, Status: 0x%2, Data length: %3")
    .arg(packet.command, 2, 16, QChar('0'))
    .arg(packet.status, 2, 16, QChar('0'))
    .arg(packet.data_len);

  if (packet.data_len > 0) {
    QString dataStr = " Data: ";
    QString asciiStr = " ASCII: ";
    for (int i = 0; i < packet.data_len; i++) {
      dataStr += QString("%1").arg(packet.data[i]);
      if (i < packet.data_len - 1) {dataStr += ",";}

      // Convert to ASCII representation
      uint8_t byte = packet.data[i];
      if (byte >= 32 && byte <= 126) {
        // Printable ASCII character
        asciiStr += QChar(byte);
      } else {
        // Non-printable character, show as hex
        asciiStr += QString("[0x%1]").arg(byte, 2, 16, QChar('0')).toUpper();
      }
    }
    logMsg += dataStr + asciiStr;
  }
  logMessage(logMsg);
}

std::vector<int16_t> PacketCalcGUI::parsePacketData(const QString & text)
{
  std::vector<int16_t> data;
  QStringList values = text.split(',', Qt::SkipEmptyParts);

  for (const QString & value : values) {
    QString trimmed = value.trimmed();
    bool ok;
    int16_t num = trimmed.toShort(&ok);
    if (ok && num >= -1 && num <= 255) {
      data.push_back(num);
      if (num == -1) {
        break;
      }
    }
  }
  return data;
}

uint8_t PacketCalcGUI::calculateCRC(
  uint8_t client_id, uint8_t command,
  const std::vector<uint8_t> & data)
{
  std::vector<uint8_t> crc_input;
  crc_input.push_back('#');
  crc_input.push_back(client_id);
  crc_input.push_back(command);
  crc_input.push_back(static_cast<uint8_t>(data.size()));
  crc_input.insert(crc_input.end(), data.begin(), data.end());

  return crc8_calculate(crc_input.data(), crc_input.size());
}

void PacketCalcGUI::updatePacketCalculation()
{
  QString packetText = ui->packetDataTextEdit->toPlainText();
  std::vector<int16_t> parsedData = parsePacketData(packetText);

  std::vector<uint8_t> validData;
  for (int16_t value : parsedData) {
    if (value == -1) {break;}
    validData.push_back(static_cast<uint8_t>(value));
  }

  uint8_t client_id = static_cast<uint8_t>(ui->clientIdSpinBox->value());
  uint8_t command = static_cast<uint8_t>(ui->CommandSpinBox->value());
  uint8_t crc = calculateCRC(client_id, command, validData);

  ui->calculatedLengthLabel->setText(QString("Calculated Length: %1").arg(validData.size()));
  ui->calculatedCrcLabel->setText(
    QString("Calculated CRC: 0x%1").arg(
      crc, 2, 16, QChar(
        '0')).toUpper());
}

void PacketCalcGUI::onPacketDataChanged()
{
  updatePacketCalculation();
}

void PacketCalcGUI::calculatePacket()
{
  updatePacketCalculation();
  logMessage("Packet calculation updated");
}

void PacketCalcGUI::sendCustomPacket()
{
  if (!isConnected) {
    QMessageBox::warning(this, "Not Connected", "Please connect to a device first.");
    return;
  }

  if (isSending) {
    QMessageBox::information(
      this, "Sending in Progress",
      "Please wait for the current packet to complete.");
    return;
  }

  QString packetText = ui->packetDataTextEdit->toPlainText();
  std::vector<int16_t> parsedData = parsePacketData(packetText);

  if (parsedData.empty()) {
    QMessageBox::warning(this, "Invalid Data", "Please enter valid packet data.");
    return;
  }

  std::vector<uint8_t> validData;
  for (int16_t value : parsedData) {
    if (value == -1) {break;}
    validData.push_back(static_cast<uint8_t>(value));
  }

  if (validData.empty()) {
    QMessageBox::warning(this, "No Data", "No valid data to send (only -1 found).");
    return;
  }

  uint8_t client_id = static_cast<uint8_t>(ui->clientIdSpinBox->value());

  Packet packet;
  packet.mode = SERIAL_MODE_HOST;
  packet.client_id = client_id;
  packet.command = static_cast<uint8_t>(ui->CommandSpinBox->value());
  packet.data_len = validData.size();

  if (packet.data_len > 0) {
    memcpy(packet.data, validData.data(), packet.data_len);
  }

  // Set sending state
  setSendingState(true);
  logMessage(
    QString("Sending packet - Command: 0x%1, Data length: %2")
    .arg(packet.command, 2, 16, QChar('0'))
    .arg(packet.data_len));

  // Blocking packet sending with 0.2s timeout
  Packet response;
  bool success = false;

  // ボタンを無効化
  ui->sendPacketButton->setEnabled(false);

  try {
    if (deviceRole == "client") {
      success = interface->sub(response);
    } else {
      success = interface->pub_sub(packet, response);
    }
  } catch (const std::exception & e) {
    logMessage(QString("Packet send error: %1").arg(e.what()));
    success = false;
  }

  // Enable the button again
  ui->sendPacketButton->setEnabled(true);

  // Immediately process the result
  setSendingState(false);
  updateResponseDisplay(response, success);

  if (success && response.is_valid) {
    logMessage(
      QString("Packet sent successfully - Response received with %1 bytes")
      .arg(response.data_len));
  } else {
    logMessage("Packet sent but no valid response received (timeout or no client)");
  }
}

void PacketCalcGUI::logMessage(const QString & message)
{
  QDateTime currentTime = QDateTime::currentDateTime();
  QString timestampedMessage = QString("[%1] %2")
    .arg(currentTime.toString("hh:mm:ss"))
    .arg(message);

  ui->logTextEdit->append(timestampedMessage);

  QTextCursor cursor = ui->logTextEdit->textCursor();
  cursor.movePosition(QTextCursor::End);
  ui->logTextEdit->setTextCursor(cursor);
}


void PacketCalcGUI::updateResponseDisplay(const Packet & response, bool success)
{
  if (success && response.is_valid) {
    ui->responseCommandLabel->setText(
      QString("Command: 0x%1").arg(
        response.command, 2, 16, QChar('0')).toUpper());
    ui->responseClientIdLabel->setText(
      QString("Client ID: %1").arg(response.client_id));
    ui->responseCrcLabel->setText(
      QString("CRC: 0x%1").arg(
        response.crc, 2, 16, QChar('0')).toUpper());

    QString dataStr = "Data: ";
    QString asciiStr = "Data (ASCII): ";
    for (int i = 0; i < response.data_len; i++) {
      dataStr += QString("%1").arg(response.data[i]);
      if (i < response.data_len - 1) {dataStr += ",";}

      // Convert to ASCII representation
      uint8_t byte = response.data[i];
      if (byte >= 32 && byte <= 126) {
        // Printable ASCII character
        asciiStr += QChar(byte);
      } else {
        // Non-printable character, show as hex
        asciiStr += QString("[0x%1]").arg(byte, 2, 16, QChar('0')).toUpper();
      }
    }
    ui->responseDataLabel->setText(dataStr);
    ui->responseDataAsciiLabel->setText(asciiStr);
  } else {
    ui->responseCommandLabel->setText("Command: TIMEOUT");
    ui->responseClientIdLabel->setText("Client ID: N/A");
    ui->responseCrcLabel->setText("CRC: N/A");
    ui->responseDataLabel->setText("Data: No response (timeout after 1s)");
    ui->responseDataAsciiLabel->setText("Data (ASCII): No response");
  }
}

void PacketCalcGUI::setSendingState(bool sending)
{
  isSending = sending;

  if (sending) {
    ui->sendPacketButton->setText("Sending...");
    ui->sendPacketButton->setEnabled(false);
    ui->responseCommandLabel->setText("Command: SENDING");
    ui->responseClientIdLabel->setText("Client ID: SENDING");
    ui->responseCrcLabel->setText("CRC: SENDING");
    ui->responseDataLabel->setText("Data: Waiting for response...");
    ui->responseDataAsciiLabel->setText("Data (ASCII): Waiting...");
  } else {
    ui->sendPacketButton->setText("Send Custom Packet");
    ui->sendPacketButton->setEnabled(isConnected);
  }

  // Update the packet calculator group enabled state
  ui->packetCalculatorGroup->setEnabled(isConnected && !isSending);
}

void PacketCalcGUI::onClientReceiveTimer()
{
  if (!isConnected || deviceRole != "client" || !interface) {
    return;
  }

  // Simple synchronous receive - no async complications
  Packet response;
  bool success = false;

  try {
    success = interface->sub(response);
  } catch (const std::exception & e) {
    logMessage(QString("Client receive error: %1").arg(e.what()));
    return;
  }

  if (success && response.is_valid) {
    onDataReceived(response);
    updateResponseDisplay(response, true);
  }
}

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);

  QString port;
  QString role = "host";
  for (int i = 1; i < argc; ++i) {
    QString arg = QString::fromUtf8(argv[i]);
    if (arg.startsWith("port:=")) {
      port = arg.mid(6);
    } else if (arg.startsWith("role:=")) {
      role = arg.mid(6);
    }
  }

  PacketCalcGUI window(port, role);
  window.show();

  return app.exec();
}

#include "packet_calc_gui.moc"
