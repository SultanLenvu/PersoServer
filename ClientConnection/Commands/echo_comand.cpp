#include "echo_comand.h"

EchoCommand::EchoCommand(const QString& name) : AbstractClientCommand(name) {
  Status = ReturnStatus::Unknown;
}

EchoCommand::~EchoCommand() {}

void EchoCommand::process(const QJsonObject& command) {
  if (command.size() != CommandSize ||
      (command["command_name"] != CommandName) || !command.contains("data")) {
    Status = ReturnStatus::SyntaxError;
    return;
  }

  Parameters.insert("data", command["data"].toString());
  Status = ReturnStatus::NoError;
}

void EchoCommand::generateResponse(QJsonObject& response) {
  response["command_name"] = CommandName;
  response["data"] = Parameters.value("data");
  response["return_status"] = QString::number(static_cast<size_t>(Status));
}

void EchoCommand::reset() {
  Parameters.clear();
  Result.clear();
  Status = ReturnStatus::Unknown;
}
