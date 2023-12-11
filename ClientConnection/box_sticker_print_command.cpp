#include "box_sticker_print_command.h"
#include "General/definitions.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

BoxStickerPrintCommand::BoxStickerPrintCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(
      this, &BoxStickerPrintCommand::printBoxSticker_signal,
      dynamic_cast<AbstractProductionDispatcher*>(
          GlobalEnvironment::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::printBoxStickerManually,
      Qt::BlockingQueuedConnection);
}

BoxStickerPrintCommand::~BoxStickerPrintCommand() {}

void BoxStickerPrintCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) ||
      !command.contains("personal_account_number")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("personal_account_number", command.value("pan").toString());

  // Запрашиваем печать бокса
  emit printBoxSticker_signal(Parameters, Status);
}

void BoxStickerPrintCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void BoxStickerPrintCommand::reset() {
  Parameters.clear();
  Status = ReturnStatus::Unknown;
}
