#include "abstract_client_connection.h"

AbstractClientConnection::AbstractClientConnection(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);
}

AbstractClientConnection::~AbstractClientConnection() {}

AbstractClientConnection::AbstractClientConnection() : QObject{nullptr} {}
