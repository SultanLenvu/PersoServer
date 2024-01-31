#include "abstract_sql_table.h"
#include "global_environment.h"
#include "log_system.h"

AbstractSqlTable::AbstractSqlTable(const QString& name) : QObject{nullptr} {
  setObjectName(name);

  connect(this, &AbstractSqlTable::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractSqlTable::~AbstractSqlTable() {}
