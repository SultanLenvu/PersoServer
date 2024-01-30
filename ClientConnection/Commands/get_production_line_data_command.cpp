#include "get_production_line_data_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

GetProductionLineDataCommand::GetProductionLineDataCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &GetProductionLineDataCommand::getProductionLineData_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject("ProductionDispatcher")),
          &AbstractProductionDispatcher::getProductinoLineData,
          Qt::BlockingQueuedConnection);
}

GetProductionLineDataCommand::~GetProductionLineDataCommand() {}

void GetProductionLineDataCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    sendLog("Получена синтаксическая ошибка.");
    return;
  }

  // Запрашиваем печать бокса
  emit getProductionLineData_signal(ProductionLineData, Status);
}

void GetProductionLineDataCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["production_line_id"] =
        ProductionLineData.value("production_line_id");
    response["production_line_login"] =
        ProductionLineData.value("production_line_login");
    response["production_line_ns"] =
        ProductionLineData.value("production_line_ns");
    response["production_line_in_process"] =
        ProductionLineData.value("production_line_in_process");
    response["today_assembled_boxes"] =
        ProductionLineData.value("today_assembled_boxes");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void GetProductionLineDataCommand::reset() {
  Parameters.clear();
  ProductionLineData.clear();
  Status = ReturnStatus::Unknown;
}
