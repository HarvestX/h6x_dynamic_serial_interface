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
#include "serial_interface.hpp"

class PacketCalcGUI : public QMainWindow
{
  Q_OBJECT

public:
  PacketCalcGUI(QWidget * parent = nullptr);
  ~PacketCalcGUI();

private slots:
  void connectToDevice();
  void disconnectFromDevice();
  void calculatePacket();
  void sendCustomPacket();
  void onPacketDataChanged();
  void onDataReceived(const Packet & packet);

private:
  void setupUI();
  void updateConnectionStatus(bool connected);
  void logMessage(const QString & message);
  void populateSerialPorts();
  std::vector<int16_t> parsePacketData(const QString & text);
  uint8_t calculateCRC(uint8_t client_id, uint8_t mode, const std::vector<uint8_t> & data);
  void updatePacketCalculation();

  // UI Components
  Ui::PacketCalcGUI * ui;

  // Communication
  serialInterface * interface;
  bool isConnected;
};

PacketCalcGUI::PacketCalcGUI(QWidget * parent)
: QMainWindow(parent), ui(new Ui::PacketCalcGUI), interface(nullptr), isConnected(false)
{
  setupUI();

  interface = new serialInterface();

  populateSerialPorts();
  updatePacketCalculation();
}

PacketCalcGUI::~PacketCalcGUI()
{
  if (interface) {
    delete interface;
  }
  delete ui;
}

void PacketCalcGUI::setupUI()
{
  ui->setupUi(this);

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
    ui->modeSpinBox, QOverload<int>::of(
      &QSpinBox::valueChanged), this, &PacketCalcGUI::onPacketDataChanged);
}

void PacketCalcGUI::populateSerialPorts()
{
  ui->portComboBox->clear();
  QDir dir("/dev");
  QStringList filters;
  // filters << "ttyACM-heye*";
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

  ui->packetCalculatorGroup->setEnabled(connected);
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
  uint8_t command = static_cast<uint8_t>(ui->modeSpinBox->value());
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
  uint8_t mode = static_cast<uint8_t>(ui->modeSpinBox->value());

  Packet packet;
  packet.mode = SERIAL_MODE_HOST;
  packet.client_id = client_id;
  packet.command = static_cast<uint8_t>(ui->modeSpinBox->value());
  packet.data_len = validData.size();

  if (packet.data_len > 0) {
    memcpy(packet.data, validData.data() + 1, packet.data_len);
  }

  Packet response;
  bool success = interface->pub_sub(packet, response);

  if (success && response.is_valid) {
    ui->responseModeLabel->setText(
      QString("Mode: 0x%1").arg(
        response.mode, 2, 16, QChar(
          '0')).toUpper());
    ui->responseCrcLabel->setText(
      QString("CRC: 0x%1").arg(
        response.crc, 2, 16, QChar(
          '0')).toUpper());

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

    logMessage(
      QString("Custom packet sent successfully - Command: 0x%1, Response length: %2")
      .arg(packet.command, 2, 16, QChar('0'))
      .arg(response.data_len));
  } else {
    ui->responseModeLabel->setText("Mode: ERROR");
    ui->responseCrcLabel->setText("CRC: ERROR");
    ui->responseDataLabel->setText("Data: No response or invalid");
    ui->responseDataAsciiLabel->setText("Data (ASCII): No response or invalid");
    logMessage("Failed to send custom packet or receive valid response");
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

int main(int argc, char * argv[])
{
  QApplication app(argc, argv);

  PacketCalcGUI window;
  window.show();

  return app.exec();
}

#include "packet_calc_gui.moc"
