#include "log_system.h"

LogSystem::LogSystem(QObject* parent) : QObject(parent) {
  LogSocket = new QUdpSocket(this);
}

LogSystem::~LogSystem() {}

void LogSystem::generate(const QString& log) {
  QTime time = QDateTime::currentDateTime().time();
  QString logMsg = time.toString("hh:mm:ss.zzz - ") + log;

  LogSocket->writeDatagram(logMsg.toUtf8(), DestIp, DestPort);
}

/*
 * Приватные методы
 */

void LogSystem::loadSettings() {
  QSettings settings;

  DestIp = settings.value("LogSystem/Destination/Ip").toString();
  if (DestIp.isNull()) {
    std::cout << QString("Invalid LogSystem destination IP. Use default %1")
                     .arg(DEFAULT_LOG_DESTINATION_IP)
                     .toUtf8()
                     .constData();
    DestIp = DEFAULT_LOG_DESTINATION_IP;
  }
  DestPort = settings.value("LogSystem/Destination/Port").toUInt();
  if (DestPort == 0) {
    std::cout << QString("Invalid LogSystem destination port. Use default %1")
                     .arg(QString::number(DEFAULT_LOG_DESTINATION_PORT))
                     .toUtf8()
                     .constData();
    DestPort = DEFAULT_LOG_DESTINATION_PORT;
  }

  DestIp = settings.value("LogSystem/Sending/Ip").toString();
  if (DestIp.isNull()) {
    std::cout << QString("Invalid LogSystem destination IP. Use default %1")
                     .arg(DEFAULT_LOG_SENDING_IP)
                     .toUtf8()
                     .constData();
    DestIp = DEFAULT_LOG_SENDING_IP;
  }
  DestPort = settings.value("LogSystem/Sending/Port").toUInt();
  if (DestPort == 0) {
    std::cout << QString("Invalid LogSystem destination port. Use default %1")
                     .arg(QString::number(DEFAULT_LOG_SENDING_PORT))
                     .toUtf8()
                     .constData();
    DestPort = DEFAULT_LOG_SENDING_PORT;
  }

  LogSocket->bind(DestIp, DestPort);
}
