#include "console_log_backend.h"

ConsoleLogBackend::ConsoleLogBackend(const QString& name) : LogBackend(name) {
  loadSettings();
}

ConsoleLogBackend::~ConsoleLogBackend() {}

void ConsoleLogBackend::writeLogLine(const QString& str) {
  if (LogEnable) {
    QTextStream(stdout) << str << "\n";
  }
}

void ConsoleLogBackend::clear() {}

void ConsoleLogBackend::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/console_log_enable").toBool();
}
