#include "shutdown_command.h"
#include "Management/global_context.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ShutdownCommand::ShutdownCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(
      this, &ShutdownCommand::shutdownProductionLine_signal,
      dynamic_cast<const AbstractProductionDispatcher*>(
          GlobalContext::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::shutdownProductionLine,
      Qt::BlockingQueuedConnection);
}

ShutdownCommand::~ShutdownCommand() {}

ReturnStatus ShutdownCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    return ReturnStatus::SyntaxError;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());

  // Запрашиваем печать бокса
  emit shutdownProductionLine_signal(Parameters, Status);

  return Status;
}

void ShutdownCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void ShutdownCommand::reset() {
  Parameters.clear();
  Status = ReturnStatus::Unknown;
}
