#include "abstract_release_system.h"
#include "global_environment.h"
#include "log_system.h"

AbstractReleaseSystem::AbstractReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;

  connect(this, &AbstractReleaseSystem::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractReleaseSystem::~AbstractReleaseSystem() {}

AbstractReleaseSystem::AbstractReleaseSystem()
    : QObject{nullptr} {}
