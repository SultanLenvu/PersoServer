#include "abstract_client_command.h"
#include "global_environment.h"
#include "log_system.h"

AbstractClientCommand::AbstractClientCommand(const QString& name)
    : ProductionLineContextOwner{name} {
  setObjectName(name);

  connect(this, &AbstractClientCommand::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractClientCommand::~AbstractClientCommand() {}

void AbstractClientCommand::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
}
