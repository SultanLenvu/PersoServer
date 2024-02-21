#include "log_system.h"
#include "console_log_backend.h"
#include "definitions.h"
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

void LogSystem::generate(const QString& log) {
  if (!LogEnable) {
    return;
  }

  QString logMsg = log;
  if (log.size() > LOG_MESSAGE_MAX_SIZE) {
    logMsg.truncate(LOG_MESSAGE_MAX_SIZE);
  }

  QString LogData = QString("%1 - %2").arg(
      QDateTime::currentDateTime().time().toString("hh:mm:ss.zzz"), logMsg);

  for (auto it = Backends.begin(); it != Backends.end(); it++) {
    (*it)->writeLogMessage(LogData);
  }
}

LogSystem::LogSystem() {}

/*
 * Приватные методы
 */

void LogSystem::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
  MessageMaxSize = settings.value("log_system/message_max_size").toInt();
}
