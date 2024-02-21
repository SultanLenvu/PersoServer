#include <QObject>

#include "log_backend.h"

LogBackend::LogBackend(const QString& name) : QObject(nullptr) {
  setObjectName(name);
}

LogBackend::~LogBackend() {}

LogBackend::LogBackend() {}
