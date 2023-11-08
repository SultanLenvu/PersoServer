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
  Tables.push_back(name);
}

void IDatabaseController::clearForeignKeys() {
  //  ForeignKeys.clear();
}

bool IDatabaseController::addRelation(const QString& table1,
                                      const QString& foreignKey,
                                      const QString& table2,
                                      const QString& primaryKey) {
  if ((!Tables.contains(table1)) || (!Tables.contains(table2))) {
    return false;
  }

  //  PrimaryKeys.insert(table2, primaryKey);
  //  ForeignKeys.insert(table1, foreignKey);
  //  Links.insert(foreignKey, primaryKey);
}

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
