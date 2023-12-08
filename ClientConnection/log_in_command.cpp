#include "launch_command.h"
#include "Management/global_context.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

LaunchCommand::LaunchCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(
      this, &LaunchCommand::launchProductionLine_signal,
      dynamic_cast<const AbstractProductionDispatcher*>(
          GlobalContext::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::launchProductionLine,
      Qt::BlockingQueuedConnection);
}

LaunchCommand::~LaunchCommand() {}

ReturnStatus LaunchCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    return ReturnStatus::SyntaxError;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());

  // Запрашиваем печать бокса
  emit launchProductionLine_signal(Parameters, Result, Status);

  return Status;
}

void LaunchCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["transponder_sn"] = Result.value("transponder_sn");
    response["transponder_pan"] = Result.value("transponder_pan");
    response["box_id"] = Result.value("box_id");
    response["pallet_id"] = Result.value("pallet_id");
    response["order_id"] = Result.value("order_id");
    response["issuer_name"] = Result.value("issuer_name");
    response["transponder_model"] = Result.value("transponder_model");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void LaunchCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
