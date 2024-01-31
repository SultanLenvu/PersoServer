#include "abstract_production_dispatcher.h"
#include "global_environment.h"
#include "log_system.h"

AbstractProductionDispatcher::AbstractProductionDispatcher(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);

  connect(this, &AbstractProductionDispatcher::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}
