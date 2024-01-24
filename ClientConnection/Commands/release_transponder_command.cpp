#include "release_transponder_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ReleaseTransponderCommand::ReleaseTransponderCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  Firmware = std::unique_ptr<QByteArray>(new QByteArray());

  connect(this, &ReleaseTransponderCommand::releaseTransponder_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::releaseTransponder,
          Qt::BlockingQueuedConnection);
}

ReleaseTransponderCommand::~ReleaseTransponderCommand() {}

void ReleaseTransponderCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  // Запрашиваем печать бокса
  emit releaseTransponder_signal(*Firmware.get(), Status);
}

void ReleaseTransponderCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["firmware"] = QString(Firmware->toBase64());
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void ReleaseTransponderCommand::reset() {
  Parameters.clear();
  Firmware->clear();
  Status = ReturnStatus::Unknown;
}
