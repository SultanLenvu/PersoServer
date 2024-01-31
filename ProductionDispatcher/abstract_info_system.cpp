#include "abstract_info_system.h"
#include "global_environment.h"
#include "log_system.h"

AbstractInfoSystem::AbstractInfoSystem(
    const QString& name,
    const std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;

  connect(this, &AbstractInfoSystem::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractInfoSystem::~AbstractInfoSystem() {}

AbstractInfoSystem::AbstractInfoSystem() : QObject{nullptr} {}
