#include "abstract_sql_table.h"

AbstractSqlTable::AbstractSqlTable(QObject* parent) : QObject{parent} {
  setObjectName("AbstractSqlTable");
}

AbstractSqlTable::~AbstractSqlTable() {}
