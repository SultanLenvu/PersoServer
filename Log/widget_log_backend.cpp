#include "widget_log_backend.h"

WidgetLogBackend::WidgetLogBackend(QObject* parent) : LogBackend(parent) {
  setObjectName("WidgetLogBackend");
  loadSettings();
}

void WidgetLogBackend::writeLogLine(const QString& str) {
  if (LogEnable) {
    emit displayLog_signal(str);
  }
}

void WidgetLogBackend::clear() {
  if (LogEnable) {
    emit clearLogDisplay_signal();
  }
}

void WidgetLogBackend::applySettings() {
  loadSettings();
}

void WidgetLogBackend::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/display_log_enable").toBool();
}
