#include "abstract_client_command.h"

AbstractClientCommand::AbstractClientCommand(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);
}

AbstractClientCommand::~AbstractClientCommand() {}

AbstractClientCommand::AbstractClientCommand() {}
