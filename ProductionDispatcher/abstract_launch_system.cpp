#include "abstract_launch_system.h"
#include "global_environment.h"
#include "log_system.h"

AbstractLaunchSystem::AbstractLaunchSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;

  connect(this, &AbstractLaunchSystem::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractLaunchSystem::~AbstractLaunchSystem() {}

AbstractLaunchSystem::AbstractLaunchSystem() : QObject{nullptr} {}
