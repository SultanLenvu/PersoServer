#include "complete_current_box_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

CompleteCurrentBoxCommand::CompleteCurrentBoxCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &CompleteCurrentBoxCommand::completeCurrentBox_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject("ProductionDispatcher")),
          &AbstractProductionDispatcher::completeBox,
          Qt::BlockingQueuedConnection);
}

CompleteCurrentBoxCommand::~CompleteCurrentBoxCommand() {}

void CompleteCurrentBoxCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    sendLog("Получена синтаксическая ошибка.");
    return;
  }

  // Запрашиваем авторизацию
  emit completeCurrentBox_signal(Status);
}

void CompleteCurrentBoxCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void CompleteCurrentBoxCommand::reset() {
  Status = ReturnStatus::Unknown;
}
