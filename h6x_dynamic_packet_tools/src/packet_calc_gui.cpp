// Copyright 2025 HarvestX Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileDialog>
#include <vector>
#include <cstring>
#include "ui_packet_calc_gui.h"

#include "h6x_dynamic_packet_handler/h6x_dynamic_packet_definitions_base.h"
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
  void setEditableLengthForTable(QTableWidget * table, int length);
  void onPacketDataChanged();
  void onDataReceived(const Packet & packet);
  void onClientReceiveTimer();
  void exportToJson();

private:
  void setupUI();
  void updateConnectionStatus(bool connected);
  void logMessage(const QString & message);
  void populateSerialPorts();
  std::vector<int16_t> parsePacketDataFromTable();
  uint8_t calculateCRC(uint8_t client_id, uint8_t mode, const std::vector<uint8_t> & data);
  void updatePacketCalculation();
  void updateResponseDisplay(const Packet & response, bool success);
  void setSendingState(bool sending);
  void get_host_status();
  void get_client_status();
  void export_packet_to_json(
    const QString & mode, uint8_t client_id, uint8_t command,
    const std::vector<uint8_t> & validData, uint8_t crc);
  void export_response_to_json();

  // UI Components
  Ui::PacketCalcGUI * ui;

  //Received packet data
  Packet receivedPacket;

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

  // Configure data tables to 8 columns x 32 rows (wrap at 8, capacity 256)
  const int kCols = 8;
  const int kRows = 32;
  ui->packetDataTableWidget->setColumnCount(kCols);
  ui->packetDataTableWidget->setRowCount(kRows);
  ui->responseDataTableWidget->setColumnCount(kCols);
  ui->responseDataTableWidget->setRowCount(kRows);

  QString title = this->windowTitle();
  if (deviceRole == "host") {
    setWindowTitle(title + " (HOST)");
  } else if (deviceRole == "client") {
    setWindowTitle(title + " (CLIENT)");
    ui->packetDataTableWidget->setEnabled(false);
    ui->responseDataTableWidget->setEnabled(false);
    ui->calculateButton->setEnabled(false);
    ui->sendPacketButton->setEnabled(false);
    ui->CommandSpinBox->setEnabled(false);
    ui->clientIdSpinBox->setEnabled(false);
  }

  ui->packetDataTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->responseDataTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

  // Set default baudrate to 115200 (index 4)
  ui->baudrateComboBox->setCurrentIndex(4);

  // Initialize
  setEditableLengthForTable(ui->responseDataTableWidget, 0);

  for (int row = 0; row < ui->packetDataTableWidget->rowCount(); ++row) {
    for (int col = 0; col < ui->packetDataTableWidget->columnCount(); ++col) {
      QTableWidgetItem * item = new QTableWidgetItem();
      item->setTextAlignment(Qt::AlignCenter);
      ui->packetDataTableWidget->setItem(row, col, item);
    }
  }

  // Connect signals
  connect(ui->connectButton, &QPushButton::clicked, this, &PacketCalcGUI::connectToDevice);
  connect(ui->disconnectButton, &QPushButton::clicked, this, &PacketCalcGUI::disconnectFromDevice);
  connect(ui->calculateButton, &QPushButton::clicked, this, &PacketCalcGUI::calculatePacket);
  connect(ui->sendPacketButton, &QPushButton::clicked, this, &PacketCalcGUI::sendCustomPacket);
  connect(ui->exportButton, &QPushButton::clicked, this, &PacketCalcGUI::exportToJson);
  connect(ui->exportButton_2, &QPushButton::clicked, this, &PacketCalcGUI::export_response_to_json);


  auto valueChangedInt = QOverload<int>::of(&QSpinBox::valueChanged);
  connect(
    ui->PacketDataLengthSpinBox, valueChangedInt, this,
    [this](int len) {
      setEditableLengthForTable(ui->packetDataTableWidget, len);
      updatePacketCalculation();
    });

  connect(
    ui->packetDataTableWidget, &QTableWidget::cellChanged, this,
    &PacketCalcGUI::onPacketDataChanged);
  connect(
    ui->clientIdSpinBox, QOverload<int>::of(
      &QSpinBox::valueChanged), this, &PacketCalcGUI::onPacketDataChanged);
  connect(
    ui->CommandSpinBox, QOverload<int>::of(
      &QSpinBox::valueChanged), this, &PacketCalcGUI::onPacketDataChanged);

  // Initialize
  setEditableLengthForTable(ui->packetDataTableWidget, 0);
  setEditableLengthForTable(ui->responseDataTableWidget, 0);
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
  QFileInfoList files = dir.entryInfoList(filters, QDir::System);
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
  int baudrate = ui->baudrateComboBox->currentText().toInt();

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

  // Store the received packet for later use
  receivedPacket = packet;

  if (packet.data_len > 0) {
    QString dataStr = " Data: ";
    for (int i = 0; i < packet.data_len; i++) {
      dataStr += QString("%1").arg(packet.data[i]);
      if (i < packet.data_len - 1) {dataStr += ",";}
    }
    logMsg += dataStr;
  }
  logMessage(logMsg);
}

std::vector<int16_t> PacketCalcGUI::parsePacketDataFromTable()
{
  std::vector<int16_t> data;

  for (int row = 0; row < ui->packetDataTableWidget->rowCount(); ++row) {
    for (int col = 0; col < ui->packetDataTableWidget->columnCount(); ++col) {
      QTableWidgetItem * item = ui->packetDataTableWidget->item(row, col);
      if (item) {
        QString text = item->text().trimmed();
        if (!text.isEmpty()) {
          bool ok = false;
          int16_t num = text.toShort(&ok);
          if (ok && num >= -1 && num < PACKET_LENGTH_MAX) {
            data.push_back(num);
            if (num == -1) {
              return data;  // -1で終了
            }
          }
        }
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
  std::vector<int16_t> parsedData = parsePacketDataFromTable();

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

void PacketCalcGUI::setEditableLengthForTable(QTableWidget * table, int length)
{
  if (!table) {return;}

  const int totalRows = table->rowCount();
  const int totalCols = table->columnCount();

  for (int row = 0; row < totalRows; ++row) {
    for (int col = 0; col < totalCols; ++col) {
      const int index = row * totalCols + col;

      QTableWidgetItem * item = table->item(row, col);
      if (!item) {
        item = new QTableWidgetItem();
        table->setItem(row, col, item);
      }

      if (index < length) {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setBackground(Qt::white);
      } else {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        item->setBackground(Qt::lightGray);
        item->setText("");
      }
    }
  }
  updatePacketCalculation();
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

  std::vector<int16_t> parsedData = parsePacketDataFromTable();

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

  Packet response;
  bool success = false;
  ui->sendPacketButton->setEnabled(false);

  try {
    if (deviceRole == "client") {
      Packet recv_pkt;
      Packet send_pkt;
      success = interface->get_serial_data(&recv_pkt, 0x23);
      if (success) {
        send_pkt = packet;
        send_pkt.mode = SERIAL_MODE_CLIENT;
        success = interface->put_serial_data(&send_pkt);
      }

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
  QTableWidget * table = ui->responseDataTableWidget;

  if (success && response.is_valid) {
    ui->responseCommandLabel->setText(
      QString("Command: 0x%1").arg(response.command, 2, 16, QChar('0')).toUpper());
    ui->responseClientIdLabel->setText(QString("Client ID: %1").arg(response.client_id));
    ui->responseCrcLabel->setText(
      QString("CRC: 0x%1").arg(response.crc, 2, 16, QChar('0')).toUpper());

    if (table) {
      const int rows = table->rowCount();
      const int cols = table->columnCount();
      const int cap = rows * cols;
      const int len = std::min<int>(response.data_len, cap);

      setEditableLengthForTable(table, len);

      table->setUpdatesEnabled(false);
      int idx = 0;
      for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
          QTableWidgetItem * item = table->item(r, c);
          if (!item) {
            item = new QTableWidgetItem();
            table->setItem(r, c, item);
          }
          if (idx < len) {
            const unsigned v = static_cast<unsigned>(response.data[idx]);
            item->setTextAlignment(Qt::AlignCenter);
            item->setText(QString::number(v));
          } else {
            item->setText("");
          }
          ++idx;
        }
      }
      table->setUpdatesEnabled(true);
    }
  } else {
    ui->responseCommandLabel->setText("Command: TIMEOUT");
    ui->responseClientIdLabel->setText("Client ID: N/A");
    ui->responseCrcLabel->setText("CRC: N/A");

    if (table) {
      setEditableLengthForTable(table, 0);

      const int rows = table->rowCount();
      const int cols = table->columnCount();
      table->setUpdatesEnabled(false);
      for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
          if (auto * item = table->item(r, c)) {item->setText("");}
        }
      }
      table->setUpdatesEnabled(true);
    }
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
    // ui->responseDataLabel->setText("Data: Waiting for response...");
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

  Packet response;
  bool success = false;

  try {
    Packet recv_pkt;
    success = interface->get_serial_data(&recv_pkt, 0x23);
    if (success) {
      response = recv_pkt;
      response.mode = SERIAL_MODE_CLIENT;
      success = interface->put_serial_data(&response);
    } else {
      return;
    }
  } catch (const std::exception & e) {
    logMessage(QString("Client receive error: %1").arg(e.what()));
    return;
  }

  if (success && response.is_valid) {
    onDataReceived(response);
    updateResponseDisplay(response, true);
  }
}

void PacketCalcGUI::exportToJson()
{
  if (deviceRole == "host") {
    get_host_status();
  } else if (deviceRole == "client") {
    get_client_status();
  }
}

void PacketCalcGUI::get_host_status()
{
  std::vector<int16_t> parsedData = parsePacketDataFromTable();
  std::vector<uint8_t> validData;
  for (int16_t value : parsedData) {
    if (value == -1) {break;}
    validData.push_back(static_cast<uint8_t>(value));
  }

  uint8_t client_id = static_cast<uint8_t>(ui->clientIdSpinBox->value());
  uint8_t command = static_cast<uint8_t>(ui->CommandSpinBox->value());
  uint8_t crc = calculateCRC(client_id, command, validData);

  export_packet_to_json("host", client_id, command, validData, crc);
}

void PacketCalcGUI::get_client_status()
{
  if (receivedPacket.data_len == 0) {
    QMessageBox::warning(this, "No Data", "No packet data received yet from host.");
    logMessage("Export failed: No received packet available.");
    return;
  }
  uint8_t client_id = receivedPacket.client_id;
  uint8_t command = receivedPacket.command;
  std::vector<uint8_t> validData(receivedPacket.data,
    receivedPacket.data + receivedPacket.data_len);
  uint8_t crc = calculateCRC(client_id, command, validData);

  export_packet_to_json("client", client_id, command, validData, crc);
}

void PacketCalcGUI::export_packet_to_json(
  const QString & mode,
  uint8_t client_id,
  uint8_t command,
  const std::vector<uint8_t> & validData,
  uint8_t crc)
{
  QDateTime currentTime = QDateTime::currentDateTimeUtc();
  qint64 unixTime = currentTime.toSecsSinceEpoch();

  QString fileName = QFileDialog::getSaveFileName(
    this, "Save Packet Data as JSON", "",
    "JSON Files (*.json)");
  if (fileName.isEmpty()) {return;}

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "File Error", "Failed to open file for writing.");
    return;
  }

  QTextStream out(&file);
  out << "{\n";
  out << "  \"header\": {\n";
  out << "    \"frame_id\": \"packet_calc_gui\",\n";
  out << "    \"stamp\": {\n";
  out << "      \"sec\": " << unixTime << ",\n";
  out << "      \"nanosec\": 0\n";
  out << "    }\n";
  out << "  },\n";
  out << "  \"mode\": \"" << mode << "\",\n";
  out << "  \"client_id\": " << static_cast<int>(client_id) << ",\n";
  out << "  \"command\": " << static_cast<int>(command) << ",\n";
  out << "  \"packet_data\": [";

  for (size_t i = 0; i < validData.size(); ++i) {
    out << static_cast<int>(validData[i]);
    if (i != validData.size() - 1) {out << ", ";}
  }

  out << "],\n";
  out << "  \"length\": " << static_cast<int>(validData.size()) << ",\n";
  out << "  \"crc\": " << static_cast<int>(crc) << "\n";
  out << "}\n";

  file.close();
  logMessage("Packet data exported to: " + fileName);
}

void PacketCalcGUI::export_response_to_json()
{
  // Check receive packet
  if (receivedPacket.data_len == 0 || !receivedPacket.is_valid) {
    QMessageBox::warning(this, "No Data", "No valid received packet available yet.");
    logMessage("Export failed: No valid received packet.");
    return;
  }

  uint8_t client_id = receivedPacket.client_id;
  uint8_t command = receivedPacket.command;

  std::vector<uint8_t> validData(receivedPacket.data,
    receivedPacket.data + receivedPacket.data_len);

  uint8_t crc = calculateCRC(client_id, command, validData);

  export_packet_to_json("response", client_id, command, validData, crc);
  logMessage("Exported last received packet as JSON.");
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
