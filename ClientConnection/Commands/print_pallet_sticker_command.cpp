#include "print_pallet_sticker_command.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"

PrintPalletStickerCommand::PrintPalletStickerCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &PrintPalletStickerCommand::printPalletSticker_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject("ProductionDispatcher")),
          &AbstractProductionDispatcher::printPalletStickerManually,
          Qt::BlockingQueuedConnection);
}

PrintPalletStickerCommand::~PrintPalletStickerCommand() {}

void PrintPalletStickerCommand::process(const QJsonObject& command) {
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
  emit printPalletSticker_signal(Parameters, Status);
}

void PrintPalletStickerCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void PrintPalletStickerCommand::reset() {
  Parameters.clear();
  Status = ReturnStatus::Unknown;
}
