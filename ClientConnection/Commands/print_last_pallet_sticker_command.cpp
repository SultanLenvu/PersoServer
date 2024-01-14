#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"
#include "print_last_pallet_sticker_command.h"

PrintLastPalletStickerCommand::PrintLastPalletStickerCommand(
    const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &PrintLastPalletStickerCommand::printLastPalletSticker_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::printLastPalletStickerManually,
          Qt::BlockingQueuedConnection);
}

PrintLastPalletStickerCommand::~PrintLastPalletStickerCommand() {}

void PrintLastPalletStickerCommand::process(const QJsonObject& command) {
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
  emit printLastPalletSticker_signal(Status);
}

void PrintLastPalletStickerCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void PrintLastPalletStickerCommand::reset() {
  Status = ReturnStatus::Unknown;
}
