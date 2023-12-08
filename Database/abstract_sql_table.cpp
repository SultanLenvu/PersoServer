#include "abstract_sql_table.h"

AbstractSqlTable::AbstractSqlTable(const QString& name) : QObject{nullptr} {
  setObjectName(name);
}

AbstractSqlTable::~AbstractSqlTable() {}
