#include <QMutexLocker>

#include "global_environment.h"
#include "log_system.h"

LogSystem::LogSystem(const QString& name) : QObject(nullptr) {
  setObjectName(name);
  loadSettings();

  UdpLogger = new UdpLogBackend(this);
  Backends << UdpLogger;

  FileLogger = new FileLogBackend(this);
  Backends << FileLogger;

  ConsoleLogger = new ConsolerLogBackend(this);
  Backends << ConsoleLogger;

  GlobalEnvironment::instance()->registerObject(this);
}

LogSystem::~LogSystem() {}

void LogSystem::clear() {
  QMutexLocker lock(&mutex);

  for (auto it = Backends.begin(); it != Backends.end(); it++) {
    (*it)->clear();
  }
}

void LogSystem::generate(const QString& log) {
  if (!LogEnable) {
    return;
  }

  QMutexLocker lock(&mutex);

  QTime time = QDateTime::currentDateTime().time();
  QString LogData = time.toString("hh:mm:ss.zzz - ") + log;
  for (auto it = Backends.begin(); it != Backends.end(); it++) {
    (*it)->writeLogLine(LogData);
  }
}

LogSystem::LogSystem() {}

/*
 * Приватные методы
 */

void LogSystem::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
}
