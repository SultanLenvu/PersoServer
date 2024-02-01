#include "abstract_box_release_system.h"
#include "global_environment.h"
#include "log_system.h"

AbstractBoxReleaseSystem::AbstractBoxReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;

  connect(this, &AbstractBoxReleaseSystem::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractBoxReleaseSystem::~AbstractBoxReleaseSystem() {}

AbstractBoxReleaseSystem::AbstractBoxReleaseSystem() {}
