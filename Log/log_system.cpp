#include "log_system.h"
#include "console_log_backend.h"
#include "file_log_backend.h"
#include "global_environment.h"
#include "udp_log_backend.h"

LogSystem::LogSystem(const QString& name) : QObject(nullptr) {
  setObjectName(name);
  loadSettings();

  Backends.push_back(
      std::shared_ptr<UdpLogBackend>(new UdpLogBackend("UdpLogBackend")));
  Backends.push_back(
      std::shared_ptr<FileLogBackend>(new FileLogBackend("FileLogBackend")));
  Backends.push_back(std::shared_ptr<ConsoleLogBackend>(
      new ConsoleLogBackend("ConsoleLogBackend")));

  GlobalEnvironment::instance()->registerObject(this);
}

LogSystem::~LogSystem() {}

void LogSystem::clear() {
  for (auto it = Backends.begin(); it != Backends.end(); it++) {
    (*it)->clear();
  }
}

void LogSystem::generate(const QString& log) {
  if (!LogEnable) {
    return;
  }

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
