#include "database_controller.h"

IDatabaseController::IDatabaseController(QObject* parent) : QObject(parent) {
  setObjectName("IDatabaseController");
  LogOption = true;
}

void IDatabaseController::sendLog(const QString& log) const {
  if (LogOption) {
    emit logging(log);
  }
}
