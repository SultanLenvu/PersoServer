#include "launch_production_line_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

LaunchProductionLineCommand::LaunchProductionLineCommand(const QString& name) : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &LaunchProductionLineCommand::launchProductionLine_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::launchProductionLine,
          Qt::BlockingQueuedConnection);
}

LaunchProductionLineCommand::~LaunchProductionLineCommand() {}

void LaunchProductionLineCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  QString tmp = command.value("login").toString();
  Context->setLogin(tmp);
  tmp = command.value("password").toString();
  Context->setPassword(tmp);

  // Запрашиваем авторизацию
  emit launchProductionLine_signal(Status);

  if (Status == ReturnStatus::NoError) {
    emit authorized();
  }
}

void LaunchProductionLineCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  //  if (Status == ReturnStatus::NoError) {
  //    response["transponder_sn"] = Result.value("transponder_sn");
  //    response["transponder_pan"] = Result.value("transponder_pan");
  //    response["box_id"] = Result.value("box_id");
  //    response["pallet_id"] = Result.value("pallet_id");
  //    response["order_id"] = Result.value("order_id");
  //    response["issuer_name"] = Result.value("issuer_name");
  //    response["transponder_model"] = Result.value("transponder_model");
  //  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void LaunchProductionLineCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
