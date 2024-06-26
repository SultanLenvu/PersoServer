#include "confirm_transponder_rerelease_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ConfirmTransponderRereleaseCommand::ConfirmTransponderRereleaseCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &ConfirmTransponderRereleaseCommand::confirmTransponderRerelease_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::confirmTransponderRerelease,
          Qt::BlockingQueuedConnection);
}

ConfirmTransponderRereleaseCommand::~ConfirmTransponderRereleaseCommand() {}

void ConfirmTransponderRereleaseCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) ||
      !command.contains("transponder_ucid") ||
      !command.contains("transponder_pan")) {
    Status = ReturnStatus::SyntaxError;
    sendLog("Получена синтаксическая ошибка.");
    return;
  }

  Parameters.insert("personal_account_number",
                    command.value("transponder_pan").toString() + "F");
  Parameters.insert("ucid", command.value("transponder_ucid").toString());

  // Подтверждение выпуска транспондера
  emit confirmTransponderRerelease_signal(Parameters, Status);
}

void ConfirmTransponderRereleaseCommand::generateResponse(QJsonObject& response) {
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

void ConfirmTransponderRereleaseCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
