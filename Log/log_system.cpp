#include "log_system.h"

LogSystem::LogSystem(QObject* parent) : QObject(parent) {
  setObjectName("LogSystem");
  loadSettings();

  WidgetLogger = new UdpLogBackend(this);
  Backends << WidgetLogger;

  FileLogger = new FileLogBackend(this);
  Backends << FileLogger;

  ConsoleLogger = new ConsolerLogBackend(this);
  Backends << ConsoleLogger;
}

LogSystem::~LogSystem() {}

LogSystem* LogSystem::instance() {
  static LogSystem Logger(nullptr);
  return &Logger;
}

void LogSystem::clear() const {
  for (QList<LogBackend*>::const_iterator it = Backends.begin();
       it != Backends.end(); it++) {
    (*it)->clear();
  }
}

void LogSystem::generate(const QString& log) const {
  QTime time = QDateTime::currentDateTime().time();
  QString LogData = time.toString("hh:mm:ss.zzz - ") + log;
  for (QList<LogBackend*>::const_iterator it = Backends.begin();
       it != Backends.end(); it++) {
    (*it)->writeLogLine(LogData);
  }
}

/*
 * Приватные методы
 */

void LogSystem::loadSettings() {}
