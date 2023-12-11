#include "last_pallet_sticker_print_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

LastPalletStickerPrintCommand::LastPalletStickerPrintCommand(
    const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &LastPalletStickerPrintCommand::printLastPalletSticker_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::printLastPalletStickerManually,
          Qt::BlockingQueuedConnection);
}

LastPalletStickerPrintCommand::~LastPalletStickerPrintCommand() {}

void LastPalletStickerPrintCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  // Запрашиваем печать бокса
  emit printLastPalletSticker_signal(Status);
}

void LastPalletStickerPrintCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void LastPalletStickerPrintCommand::reset() {
  Status = ReturnStatus::Unknown;
}
