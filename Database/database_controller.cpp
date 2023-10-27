#include "database_controller.h"

IDatabaseController::IDatabaseController(QObject* parent) : QObject(parent) {
  setObjectName("IDatabaseController");
  LogEnable = true;
}

void IDatabaseController::sendLog(const QString& log) const {
  if (LogEnable) {
    emit logging(QString("%1 - %2").arg(objectName(), log));
  }
}
