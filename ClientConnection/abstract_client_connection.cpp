#include "abstract_client_connection.h"
#include "abstract_production_dispatcher.h"
#include "global_environment.h"
#include "log_system.h"

AbstractClientConnection::AbstractClientConnection(const QString& name)
    : ProductionContextOwner{name} {
  connectDependencies();
}

AbstractClientConnection::~AbstractClientConnection() {}

void AbstractClientConnection::connectDependencies() {
  connect(this, &AbstractClientConnection::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
  connect(this, &AbstractClientConnection::shutdownProductionLine_signal,
          dynamic_cast<AbstractProductionDispatcher*>(
              GlobalEnvironment::instance()->getObject("ProductionDispatcher")),
          &AbstractProductionDispatcher::shutdownProductionLine,
          Qt::BlockingQueuedConnection);
}
