#include "abstract_sql_database.h"
#include "global_environment.h"
#include "log_system.h"

AbstractSqlDatabase::AbstractSqlDatabase(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);

  QObject::connect(this, &AbstractSqlDatabase::logging,
                   dynamic_cast<LogSystem*>(
                       GlobalEnvironment::instance()->getObject("LogSystem")),
                   &LogSystem::generate);
}

AbstractSqlDatabase::~AbstractSqlDatabase() {}
