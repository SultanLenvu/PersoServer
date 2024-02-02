#include "abstract_transponder_release_system.h"
#include "global_environment.h"
#include "log_system.h"

AbstractTransponderReleaseSystem::AbstractTransponderReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;

  connect(this, &AbstractTransponderReleaseSystem::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractTransponderReleaseSystem::~AbstractTransponderReleaseSystem() {}

AbstractTransponderReleaseSystem::AbstractTransponderReleaseSystem()
    : QObject{nullptr} {}