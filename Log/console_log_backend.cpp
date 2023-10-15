#include "console_log_backend.h"

ConsolerLogBackend::ConsolerLogBackend(QObject *parent) : LogBackend(parent)
{
  setObjectName("ConsolerLogBackend");

  loadSettings();
}

ConsolerLogBackend::~ConsolerLogBackend() {}

void ConsolerLogBackend::writeLogLine(const QString& str) {
  if (LogEnable) {
    QTextStream(stdout) << str << "\n";
  }
}

void ConsolerLogBackend::clear() {}

void ConsolerLogBackend::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/console_log_enable").toBool();
}
