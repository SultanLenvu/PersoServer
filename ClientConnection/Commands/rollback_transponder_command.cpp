#include "rollback_transponder_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

RollbackTransponderCommand::RollbackTransponderCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &RollbackTransponderCommand::rollbackTransponder_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::rollbackTransponder,
          Qt::BlockingQueuedConnection);
}

RollbackTransponderCommand::~RollbackTransponderCommand() {}

void RollbackTransponderCommand::process(const QJsonObject& command) {
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

  // Запрашиваем печать бокса
  emit rollbackTransponder_signal(Status);
}

void RollbackTransponderCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

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

void RollbackTransponderCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
