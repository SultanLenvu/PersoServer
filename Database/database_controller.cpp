#include "database_controller.h"

IDatabaseController::IDatabaseController(QObject* parent) : QObject(parent) {
  setObjectName("IDatabaseController");
  LogEnable = true;
}

IDatabaseController::~IDatabaseController() {}

void IDatabaseController::clearTables() {
  Tables.clear();
}

void IDatabaseController::addTable(const QString& name) {
  Tables.append(name);
}

void IDatabaseController::clearForeignKeys() {}

void IDatabaseController::addForeignKeys(const QString& table1,
                                         const QString& foreignKey,
                                         const QString& table2,
                                         const QString& primaryKey) {}

Qt::SortOrder IDatabaseController::getCurrentOrder() const {
  return CurrentOrder;
}

void IDatabaseController::setCurrentOrder(Qt::SortOrder newCurrentOrder) {
  CurrentOrder = newCurrentOrder;
}

uint32_t IDatabaseController::getRecordsLimit() const {
  return RecordsLimit;
}

void IDatabaseController::setRecordsLimit(uint32_t newRecordsLimit) {
  RecordsLimit = newRecordsLimit;
}

void IDatabaseController::sendLog(const QString& log) const {
  if (LogEnable) {
    emit const_cast<IDatabaseController*>(this)->logging(
        QString("%1 - %2").arg(objectName(), log));
  }
}
