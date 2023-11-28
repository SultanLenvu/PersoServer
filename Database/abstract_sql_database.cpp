#include "abstract_sql_database.h"

AbstractSqlDatabase::AbstractSqlDatabase(QObject* parent) : QObject{parent} {
  setObjectName("AbstractSqlDatabase");
}

AbstractSqlDatabase::~AbstractSqlDatabase() {}
