#include "print_last_box_sticker_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

PrintLastBoxStickerCommand::PrintLastBoxStickerCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &PrintLastBoxStickerCommand::printLastBoxSticker_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::printLastBoxStickerManually,
          Qt::BlockingQueuedConnection);
}

PrintLastBoxStickerCommand::~PrintLastBoxStickerCommand() {}

void PrintLastBoxStickerCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName)) {
    Status = ReturnStatus::SyntaxError;
    sendLog("Получена синтаксическая ошибка.");
    return;
  }

  // Запрашиваем печать бокса
  emit printLastBoxSticker_signal(Status);
}

void PrintLastBoxStickerCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void PrintLastBoxStickerCommand::reset() {
  Status = ReturnStatus::Unknown;
}
