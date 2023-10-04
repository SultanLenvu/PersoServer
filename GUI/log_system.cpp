#include "log_system.h"

#include "Logging/log_backend.h"

LogSystem::LogSystem(QObject* parent) : QObject(parent) {
  EnableIndicator = true;
}

LogSystem::~LogSystem() {}

void LogSystem::clear() {
  /* emit requestClearDisplayLog(); */
  for (QList<LogBackend*>::iterator it = backends.begin(); it != backends.end();
      it++)
    (*it)->clear();
}

void LogSystem::setEnable(bool option) {
  EnableIndicator = option;
}

void LogSystem::addBackend(LogBackend *backend)
{
  backends << backend;
}

void LogSystem::removeBackend(LogBackend *backend)
{
  backends.removeOne(backend);
}

void LogSystem::generate(const QString& log) {
  if (!EnableIndicator) {
    return;
  }

  QTime time = QDateTime::currentDateTime().time();
  QString LogData = time.toString("hh:mm:ss.zzz - ") + log;
  /* emit requestDisplayLog(LogData); */
  for (QList<LogBackend*>::iterator it = backends.begin(); it != backends.end();
      it++)
    (*it)->writeLogLine(LogData);
}

/*
 * Приватные методы
 */
