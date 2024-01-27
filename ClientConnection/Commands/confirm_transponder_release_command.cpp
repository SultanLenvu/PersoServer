#include "confirm_transponder_release_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ConfirmTransponderReleaseCommand::ConfirmTransponderReleaseCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &ConfirmTransponderReleaseCommand::confirmTransponderRelease_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "ProductionDispatcher")),
          &AbstractProductionDispatcher::confirmTransponderRelease,
          Qt::BlockingQueuedConnection);
}

ConfirmTransponderReleaseCommand::~ConfirmTransponderReleaseCommand() {}

void ConfirmTransponderReleaseCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) ||
      !command.contains("transponder_ucid")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("transponder_ucid",
                    command.value("transponder_ucid").toString());

  // Подтверждение выпуска транспондера
  emit confirmTransponderRelease_signal(Parameters, Status);
}

void ConfirmTransponderReleaseCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void ConfirmTransponderReleaseCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
