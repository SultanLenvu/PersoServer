#include "get_current_box_data_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

GetCurrentBoxDataCommand::GetCurrentBoxDataCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &GetCurrentBoxDataCommand::getCurrentBoxData_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject("ProductionDispatcher")),
          &AbstractProductionDispatcher::getCurrentBoxData,
          Qt::BlockingQueuedConnection);
}

GetCurrentBoxDataCommand::~GetCurrentBoxDataCommand() {}

void GetCurrentBoxDataCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  // Запрашиваем авторизацию
  emit getCurrentBoxData_signal(BoxData, Status);
}

void GetCurrentBoxDataCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["box_id"] = BoxData.value("box_id");
    response["box_in_process"] = BoxData.value("box_in_process");
    response["box_quantity"] = BoxData.value("box_quantity");

    response["box_assembled_units"] = BoxData.value("box_assembled_units");
    response["box_assembling_start"] = BoxData.value("box_assembling_start");
    response["box_assembling_end"] = BoxData.value("box_assembling_end");

    response["first_transponder_sn"] = BoxData.value("first_transponder_sn");
    response["last_transponder_sn"] = BoxData.value("last_transponder_sn");

    response["pallet_id"] = BoxData.value("pallet_id");
    response["production_line_id"] = BoxData.value("production_line_id");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void GetCurrentBoxDataCommand::reset() {
  BoxData.clear();
  Status = ReturnStatus::Unknown;
}
