#include "rerelease_confirm_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

RereleaseConfirmCommand::RereleaseConfirmCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &RereleaseConfirmCommand::confirmRerelease_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::confirmTransponderRerelease,
          Qt::BlockingQueuedConnection);
}

RereleaseConfirmCommand::~RereleaseConfirmCommand() {}

void RereleaseConfirmCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password") || !command.contains("transpoder_ucid") ||
      !command.contains("transpoder_pan")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());
  Parameters.insert("personal_account_number",
                    command.value("transpoder_pan").toString());
  Parameters.insert("ucid", command.value("transpoder_ucid").toString());

  // Подтверждение выпуска транспондера
  emit confirmRerelease_signal(Parameters, Status);
}

void RereleaseConfirmCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

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

void RereleaseConfirmCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
