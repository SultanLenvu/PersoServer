#include "abstract_client_connection.h"
#include "global_environment.h"
#include "log_system.h"

AbstractClientConnection::AbstractClientConnection(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);

  connect(this, &AbstractClientConnection::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractClientConnection::~AbstractClientConnection() {}

AbstractClientConnection::AbstractClientConnection() : QObject{nullptr} {}
