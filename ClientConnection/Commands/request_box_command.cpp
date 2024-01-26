#include "request_box_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

RequestBoxCommand::RequestBoxCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &RequestBoxCommand::requestBox_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::requestBox,
          Qt::BlockingQueuedConnection);
}

RequestBoxCommand::~RequestBoxCommand() {}

void RequestBoxCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  // Запрашиваем авторизацию
  emit requestBox_signal(Status);
}

void RequestBoxCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void RequestBoxCommand::reset() {
  Status = ReturnStatus::Unknown;
}