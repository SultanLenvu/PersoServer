#include "log_system.h"

GlobalLogSystem::GlobalLogSystem(QObject* parent) : QObject(parent) {
  EnableIndicator = true;
}

GlobalLogSystem::~GlobalLogSystem() {}

void GlobalLogSystem::clear() {
  emit clearLogDisplayRequest();
}

void GlobalLogSystem::setEnable(bool option) {
  EnableIndicator = option;
}

void GlobalLogSystem::dtrLogging(const QString& log) {
  generalLogging(QString("DTR"), log);
}

void GlobalLogSystem::managerLogging(const QString& log) {
  generalLogging(QString("Manager"), log);
}

/*
 * Приватные методы
 */

void GlobalLogSystem::generalLogging(QString sender, QString log) {
  if (EnableIndicator == true) {
    QTime time = QDateTime::currentDateTime().time();
    QString LogData =
        time.toString("hh:mm:ss.zzz - ") + sender + QString(" - ") + log;
    emit displayLogRequest(LogData);
  }
}
