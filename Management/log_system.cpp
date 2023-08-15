#include "log_system.h"

LogSystem::LogSystem(QObject* parent) : QObject(parent) {
  EnableIndicator = true;
}

LogSystem::~LogSystem() {}

void LogSystem::clear() {
  emit requestClearDisplayLog();
}

void LogSystem::setEnable(bool option) {
  EnableIndicator = option;
}

void LogSystem::generate(const QString& log) {
  if (!EnableIndicator) {
    return;
  }

  QTime time = QDateTime::currentDateTime().time();
  QString LogData = time.toString("hh:mm:ss.zzz - ") + log;
  emit requestDisplayLog(LogData);
}

/*
 * Приватные методы
 */
