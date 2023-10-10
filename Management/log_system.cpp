#include "log_system.h"

LogSystem::LogSystem(QObject* parent) : QObject(parent) {
  setObjectName("LogSystem");

  LogSocket = new QUdpSocket(this);

  loadSettings();
}

LogSystem::~LogSystem() {}

void LogSystem::generate(const QString& log) const {
  QString udpMsg = "PersoServer - " + log;
  writeToUdp(udpMsg);

  QTime time = QDateTime::currentDateTime().time();
  QString consoleMsg = time.toString("hh:mm:ss.zzz - ") + log;

  writeToConsole(consoleMsg);
}

/*
 * Приватные методы
 */

void LogSystem::loadSettings() {
  QSettings settings;

  if (settings.value("log_system/console_ouput_enable").isValid()) {
    ConsoleOutputEnable =
        settings.value("log_system/console_ouput_enable").toBool();
  } else {
    ConsoleOutputEnable = true;
  }

  if (settings.value("log_system/udp_ouput_enable").isValid()) {
    UdpOutputEnable = settings.value("log_system/udp_ouput_enable").toBool();
  } else {
    UdpOutputEnable = true;
  }

  DestIp = settings.value("log_system/udp_destination_ip").toString();
  if (DestIp.isNull()) {
    writeToConsole(QString("Invalid LogSystem destination IP. Use default %1.")
                       .arg(DEFAULT_LOG_DESTINATION_IP));
    DestIp = DEFAULT_LOG_DESTINATION_IP;
  }
  DestPort = settings.value("log_system/udp_destination_port").toUInt();
  if (DestPort == 0) {
    writeToConsole(
        QString("Invalid LogSystem destination port. Use default %1.")
            .arg(QString::number(DEFAULT_LOG_DESTINATION_PORT)));
    DestPort = DEFAULT_LOG_DESTINATION_PORT;
  }

  generate(QString("Вывод логов в консоль включен."));
  generate(QString("Отправка логов по UDP включена."));
}

void LogSystem::writeToConsole(const QString& log) const {
  if (ConsoleOutputEnable) {
    QTextStream consoleOutput(stdout);
    consoleOutput << log << "\n";
  }
}

void LogSystem::writeToUdp(const QString& log) const {
  if (UdpOutputEnable) {
    LogSocket->writeDatagram(log.toUtf8(), DestIp, DestPort);
  }
}
