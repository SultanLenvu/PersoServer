#include "shutdown_production_line_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ShutdownProductionLineCommand::ShutdownProductionLineCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(
      this, &ShutdownProductionLineCommand::shutdownProductionLine_signal,
      dynamic_cast<AbstractProductionDispatcher*>(
          GlobalEnvironment::instance()->getObject("ProductionDispatcher")),
      &AbstractProductionDispatcher::shutdownProductionLine,
      Qt::BlockingQueuedConnection);
}

ShutdownProductionLineCommand::~ShutdownProductionLineCommand() {}

void ShutdownProductionLineCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  // Запрашиваем печать бокса
  emit shutdownProductionLine_signal(Status);
}

void ShutdownProductionLineCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void ShutdownProductionLineCommand::reset() {
  Parameters.clear();
  Status = ReturnStatus::Unknown;
}
