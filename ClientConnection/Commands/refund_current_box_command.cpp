#include "refund_current_box_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

RefundCurrentBoxCommand::RefundCurrentBoxCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &RefundCurrentBoxCommand::refundCurrentBox_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::refundBox,
          Qt::BlockingQueuedConnection);
}

RefundCurrentBoxCommand::~RefundCurrentBoxCommand() {}

void RefundCurrentBoxCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  if (!Context->isLaunched()) {
    sendLog("Команда не может быть выполнена без авторизации.");
    Status = ReturnStatus::UnauthorizedRequest;
    return;
  }

  // Запрашиваем авторизацию
  emit refundCurrentBox_signal(Status);
}

void RefundCurrentBoxCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void RefundCurrentBoxCommand::reset() {
  Status = ReturnStatus::Unknown;
}
