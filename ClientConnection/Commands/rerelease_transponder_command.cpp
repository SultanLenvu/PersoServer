#include "rerelease_transponder_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

RereleaseTransponderCommand::RereleaseTransponderCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  Firmware = std::unique_ptr<QByteArray>(new QByteArray());

  connect(this, &RereleaseTransponderCommand::rereleaseTransponder_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::rereleaseTransponder,
          Qt::BlockingQueuedConnection);
}

RereleaseTransponderCommand::~RereleaseTransponderCommand() {}

void RereleaseTransponderCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) ||
      !command.contains("transpoder_pan")) {
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
  emit rereleaseTransponder_signal(Parameters, *Firmware.get(), Status);
}

void RereleaseTransponderCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["firmware"] = QString(*Firmware);
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void RereleaseTransponderCommand::reset() {
  Parameters.clear();
  Firmware->clear();
  Status = ReturnStatus::Unknown;
}
