#include "perso_client_connection_builder.h"

PersoClientConnectionBuilder::PersoClientConnectionBuilder()
    : QObject(nullptr) {
  // Пока никакие объекты не созданы
  Client = nullptr;
}

PersoClientConnection* PersoClientConnectionBuilder::buildedObject() const {
  return Client;
}

void PersoClientConnectionBuilder::build(uint32_t id,
                                         qintptr socketDescriptor) {
  //  Client = new PersoClientConnection(id, socketDescriptor);

  emit completed();
}
