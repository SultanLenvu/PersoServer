#include "abstract_client_command.h"
#include "global_environment.h"
#include "log_system.h"

AbstractClientCommand::AbstractClientCommand(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);

  connect(this, &AbstractClientCommand::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractClientCommand::~AbstractClientCommand() {}

std::shared_ptr<ProductionContext> AbstractClientCommand::context() {
  return Context;
}

void AbstractClientCommand::setContext(
    std::shared_ptr<ProductionContext> context) {
  Context = context;
}

void AbstractClientCommand::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
}

AbstractClientCommand::AbstractClientCommand() {}
