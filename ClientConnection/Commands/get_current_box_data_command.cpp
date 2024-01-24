#include "get_current_box_data_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

GetCurrentBoxDataCommand::GetCurrentBoxDataCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  //  connect(this, &GetCurrentBoxDataCommand::getCurrentBoxData_signal,
  //          dynamic_cast<AbstractProductionDispatcher*>(
  //              GlobalEnvironment::instance()->getObject(
  //                  "ProductionDispatcher")),
  //          &AbstractProductionDispatcher::getCurrentBoxData,
  //          Qt::BlockingQueuedConnection);
}

GetCurrentBoxDataCommand::~GetCurrentBoxDataCommand() {}

void GetCurrentBoxDataCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  // Запрашиваем авторизацию
  //  emit getCurrentBoxData_signal(CurrentBoxData, Status);
}

void GetCurrentBoxDataCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    Context->addBoxDataToJson(response);
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void GetCurrentBoxDataCommand::reset() {
  CurrentBoxData.clear();
  Status = ReturnStatus::Unknown;
}
