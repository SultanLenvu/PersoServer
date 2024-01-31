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
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::rereleaseTransponder,
          Qt::BlockingQueuedConnection);
}

RereleaseTransponderCommand::~RereleaseTransponderCommand() {}

void RereleaseTransponderCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) ||
      !command.contains("transponder_pan")) {
    Status = ReturnStatus::SyntaxError;
    sendLog("Получена синтаксическая ошибка.");
    return;
  }

  Parameters.insert("personal_account_number",
                    command.value("transponder_pan").toString() + "F");

  // Запрашиваем печать бокса
  emit rereleaseTransponder_signal(Parameters, *Firmware.get(), Status);
}

void RereleaseTransponderCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["transponder_firmware"] = QString(Firmware->toBase64());
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void RereleaseTransponderCommand::reset() {
  Parameters.clear();
  Firmware->clear();
  Status = ReturnStatus::Unknown;
}
