#include "udp_log_backend.h"

UdpLogBackend::UdpLogBackend(const QString& name) : LogBackend(name) {
  loadSettings();

  LogSocket = new QUdpSocket(this);
}

void UdpLogBackend::writeLogMessage(const QString& str) {
  if (LogEnable) {
    LogSocket->writeDatagram(str.toUtf8(), DestIp, DestPort);
  }
}

void UdpLogBackend::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/udp_log_enable").toBool();
  DestIp = QHostAddress(
      settings.value("log_system/udp_destination_ip").toString());
  DestPort = settings.value("log_system/udp_destination_port").toUInt();
}
