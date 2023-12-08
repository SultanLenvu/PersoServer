#include "transponder_rerelease_command.h"
#include "Management/global_context.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

TransponderRereleaseCommand::TransponderRereleaseCommand(
    const QString& name,
    std::shared_ptr<AbstractFirmwareGenerationSystem> generator)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  Firmware = std::unique_ptr<QByteArray>(new QByteArray());
  Generator = generator;

  connect(
      this, &TransponderRereleaseCommand::rereleaseTransponder_signal,
      dynamic_cast<const AbstractProductionDispatcher*>(
          GlobalContext::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::rereleaseTransponder,
      Qt::BlockingQueuedConnection);
}

TransponderRereleaseCommand::~TransponderRereleaseCommand() {}

ReturnStatus TransponderRereleaseCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    return ReturnStatus::SyntaxError;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());
  Parameters.insert("personal_account_number", command.value("pan").toString());

  // Запрашиваем печать бокса
  emit rereleaseTransponder_signal(Parameters, Result, Status);

  // Генерируем прошивку
  if (!Generator->generate(Result, *Firmware)) {
    Status = ReturnStatus::FirmwareGenerationError;
  }

  return Status;
}

void TransponderRereleaseCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["firmware"] = QString::fromUtf8(Firmware->toBase64());
    response["transponder_sn"] = Result.value("transponder_sn");
    response["transponder_pan"] = Result.value("transponder_pan");
    response["box_id"] = Result.value("box_id");
    response["pallet_id"] = Result.value("pallet_id");
    response["order_id"] = Result.value("order_id");
    response["issuer_name"] = Result.value("issuer_name");
    response["transponder_model"] = Result.value("transponder_model");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void TransponderRereleaseCommand::reset() {
  Parameters.clear();
  Result.clear();
  Firmware->clear();
  Status = ReturnStatus::Unknown;
}
