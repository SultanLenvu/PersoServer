#include "log_system.h"

LogSystem::LogSystem(QObject* parent) : QObject(parent) {
  setObjectName("LogSystem");
  loadSettings();

  LogSocket = new QUdpSocket(this);
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

  ConsoleOutputEnable =
      settings.value("log_system/console_ouput_enable").toBool();

  UdpOutputEnable = settings.value("log_system/udp_ouput_enable").toBool();
  DestIp = settings.value("log_system/udp_destination_ip").toString();
  DestPort = settings.value("log_system/udp_destination_port").toUInt();

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
