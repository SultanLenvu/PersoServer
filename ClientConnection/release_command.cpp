#include "release_command.h"
#include "Management/global_context.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

ReleaseCommand::ReleaseCommand(const QString& name)
    : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;

  Firmware = std::unique_ptr<QByteArray>(new QByteArray());

  connect(
      this, &ReleaseCommand::release_signal,
      dynamic_cast<const AbstractProductionDispatcher*>(
          GlobalContext::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::releaseTransponder,
      Qt::BlockingQueuedConnection);
}

ReleaseCommand::~ReleaseCommand() {}

void ReleaseCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("login") ||
      !command.contains("password")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("login", command.value("login").toString());
  Parameters.insert("password", command.value("password").toString());

  // Запрашиваем печать бокса
  emit release_signal(Parameters, Result, Status);
}

void ReleaseCommand::generateResponse(QJsonObject& response) {
  response["response_name"] = CommandName;

  if (Status == ReturnStatus::NoError) {
    response["firmware"] = Result.value("firmware");
  }

  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void ReleaseCommand::reset() {
  Parameters.clear();
  Result.clear();
  Firmware->clear();
  Status = ReturnStatus::Unknown;
}
