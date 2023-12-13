#include "release_confirm_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ReleaseConfirmCommand::ReleaseConfirmCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &ReleaseConfirmCommand::confirmRelease_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::confirmTransponderRelease,
          Qt::BlockingQueuedConnection);
}

ReleaseConfirmCommand::~ReleaseConfirmCommand() {}

void ReleaseConfirmCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password") || !command.contains("transponder_ucid")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());
  Parameters.insert("ucid", command.value("transponder_ucid").toString());

  // Подтверждение выпуска транспондера
  emit confirmRelease_signal(Parameters, Status);
}

void ReleaseConfirmCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void ReleaseConfirmCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
