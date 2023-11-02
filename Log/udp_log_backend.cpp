#include "udp_log_backend.h"

UdpLogBackend::UdpLogBackend(QObject* parent) : LogBackend(parent) {
  setObjectName("UdpLogBackend");
  loadSettings();

  LogSocket = new QUdpSocket(this);
}

void UdpLogBackend::writeLogLine(const QString& str) {
  if (LogEnable) {
    LogSocket->writeDatagram(str.toUtf8(), DestIp, DestPort);
  }
}

void UdpLogBackend::clear() {}

void UdpLogBackend::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/udp_log_enable").toBool();
  DestIp = QHostAddress(
      settings.value("log_system/udp_destination_ip").toString());
  DestPort = settings.value("log_system/udp_destination_port").toUInt();
}
