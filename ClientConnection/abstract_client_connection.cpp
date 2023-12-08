#include "abstract_client_connection.h"
#include "Management/global_context.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

AbstractClientConnection::AbstractClientConnection(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);

  connect(
      this, &AbstractClientConnection::logOut_signal,
      dynamic_cast<const AbstractProductionDispatcher*>(
          GlobalContext::instance()->getObject("GeneralProductionDispatcher")),
      &AbstractProductionDispatcher::shutdownProductionLine,
      Qt::BlockingQueuedConnection);
}

AbstractClientConnection::~AbstractClientConnection() {}

AbstractClientConnection::AbstractClientConnection() : QObject{nullptr} {}
