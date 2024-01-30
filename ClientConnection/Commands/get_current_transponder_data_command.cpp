#include "get_current_transponder_data_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

GetCurrentTransponderDataCommand::GetCurrentTransponderDataCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this,
          &GetCurrentTransponderDataCommand::getCurrentTransponderData_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::getCurrentTransponderData,
          Qt::BlockingQueuedConnection);
}

GetCurrentTransponderDataCommand::~GetCurrentTransponderDataCommand() {}

void GetCurrentTransponderDataCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    sendLog("Получена синтаксическая ошибка.");
    return;
  }

  // Запрашиваем печать бокса
  emit getCurrentTransponderData_signal(TransponderData, Status);
}

void GetCurrentTransponderDataCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["transponder_sn"] = TransponderData.value("transponder_sn");
    response["transponder_pan"] = TransponderData.value("transponder_pan");
    response["transponder_ucid"] = TransponderData.value("transponder_ucid");
    response["transponder_release_counter"] =
        TransponderData.value("transponder_release_counter");
    response["box_id"] = TransponderData.value("box_id");
    response["issuer_name"] = TransponderData.value("issuer_name");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void GetCurrentTransponderDataCommand::reset() {
  Parameters.clear();
  TransponderData.clear();
  Status = ReturnStatus::Unknown;
}
