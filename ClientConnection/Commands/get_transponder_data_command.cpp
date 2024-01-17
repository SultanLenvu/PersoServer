#include "get_transponder_data_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

GetTransponderDataCommand::GetTransponderDataCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &GetTransponderDataCommand::getTransponderData_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::getTransponderData,
          Qt::BlockingQueuedConnection);
}

GetTransponderDataCommand::~GetTransponderDataCommand() {}

void GetTransponderDataCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) ||
      command.contains("transponder_pan")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  if (!Context->isLaunched()) {
    sendLog("Команда не может быть выполнена без авторизации.");
    Status = ReturnStatus::UnauthorizedRequest;
    return;
  }

  Parameters.insert("personal_account_number",
                    command.value("transpoder_pan").toString() + "F");

  // Запрашиваем печать бокса
  emit getTransponderData_signal(Parameters, Result, Status);
}

void GetTransponderDataCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

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

void GetTransponderDataCommand::reset() {
  Parameters.clear();
  Status = ReturnStatus::Unknown;
}
