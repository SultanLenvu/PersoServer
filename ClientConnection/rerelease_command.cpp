#include "rerelease_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

RereleaseCommand::RereleaseCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  Firmware = std::unique_ptr<QByteArray>(new QByteArray());

  connect(
      this, &RereleaseCommand::rerelease_signal,
      dynamic_cast<AbstractProductionDispatcher*>(
          GlobalEnvironment::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::rereleaseTransponder,
      Qt::BlockingQueuedConnection);
}

RereleaseCommand::~RereleaseCommand() {}

void RereleaseCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password") || !command.contains("transpoder_pan")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());
  Parameters.insert("personal_account_number",
                    command.value("transpoder_pan").toString());

  // Запрашиваем печать бокса
  emit rerelease_signal(Parameters, Result, Status);
}

void RereleaseCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["firmware"] = Result.value("firmware");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void RereleaseCommand::reset() {
  Parameters.clear();
  Result.clear();
  Firmware->clear();
  Status = ReturnStatus::Unknown;
}
