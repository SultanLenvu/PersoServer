#include "get_transponder_data_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

GetTransponderDataCommand::GetTransponderDataCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &GetTransponderDataCommand::getTransponderData_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
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

  Parameters.insert("personal_account_number",
                    command.value("transpoder_pan").toString() + "F");

  // Запрашиваем печать бокса
  emit getTransponderData_signal(Parameters, Result, Status);
}

void GetTransponderDataCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["transponder_sn"] = Context->transponder().get("transponder_sn");
    response["transponder_pan"] = Context->transponder().get("transponder_pan");
    response["transponder_ucid"] =
        Context->transponder().get("transponder_ucid");
    response["transponder_release_counter"] =
        Context->transponder().get("transponder_release_counter");
    response["box_id"] = Context->transponder().get("box_id");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void GetTransponderDataCommand::reset() {
  Parameters.clear();
  Status = ReturnStatus::Unknown;
}
