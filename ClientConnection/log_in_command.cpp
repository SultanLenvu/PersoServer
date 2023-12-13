#include "log_in_command.h"
#include "Management/global_environment.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

LogInCommand::LogInCommand(const QString& name) : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  connect(this, &LogInCommand::logIn_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject(
                  "GeneralProductionDispatcher")),
          &AbstractProductionDispatcher::launchProductionLine,
          Qt::BlockingQueuedConnection);
}

LogInCommand::~LogInCommand() {}

void LogInCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());

  // Запрашиваем авторизацию
  emit logIn_signal(Parameters, Status);

  if (Status == ReturnStatus::NoError) {
    emit authorized(Parameters.value("login"), Parameters.value("password"));
  }
}

void LogInCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

  //  if (Status == ReturnStatus::NoError) {
  //    response["transponder_sn"] = Result.value("transponder_sn");
  //    response["transponder_pan"] = Result.value("transponder_pan");
  //    response["box_id"] = Result.value("box_id");
  //    response["pallet_id"] = Result.value("pallet_id");
  //    response["order_id"] = Result.value("order_id");
  //    response["issuer_name"] = Result.value("issuer_name");
  //    response["transponder_model"] = Result.value("transponder_model");
  //  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void LogInCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
