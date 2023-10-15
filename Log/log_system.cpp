#include <QDateTime>
#include <QDir>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "log_system.h"

#include "Log/log_backend.h"
#include "Log/udp_log_backend.h"
#include "Log/udp_log_backend.h"

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

void LogSystem::applySettings() {
  generate("LogSystem - Применение новых настроек. ");
  loadSettings();
}

/*
 * Приватные методы
 */

void LogSystem::loadSettings() {}
