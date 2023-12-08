#include "abstract_sql_database.h"

AbstractSqlDatabase::AbstractSqlDatabase(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);
}

AbstractSqlDatabase::~AbstractSqlDatabase() {}
