#include "last_box_sticker_print_command.h"
#include "Management/global_context.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

LastBoxStickerPrintCommand::LastBoxStickerPrintCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(
      this, &LastBoxStickerPrintCommand::printLastBoxSticker_signal,
      dynamic_cast<const AbstractProductionDispatcher*>(
          GlobalContext::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::printLastBoxStickerManually,
      Qt::BlockingQueuedConnection);
}

LastBoxStickerPrintCommand::~LastBoxStickerPrintCommand() {}

ReturnStatus LastBoxStickerPrintCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    return ReturnStatus::SyntaxError;
  }

  // Запрашиваем печать бокса
  emit printLastBoxSticker_signal(Status);

  return Status;
}

void LastBoxStickerPrintCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void LastBoxStickerPrintCommand::reset() {
  Status = ReturnStatus::Unknown;
}
