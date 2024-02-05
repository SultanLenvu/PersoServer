#include "production_dispatcher_context.h"

ProductionDispatcherContext::ProductionDispatcherContext() {
  Pallets[""] = SqlQueryValues();
}

ProductionDispatcherContext::~ProductionDispatcherContext() {}

void ProductionDispatcherContext::addPallet(const SqlQueryValues& pallet) {
  Pallets[pallet.get("id")] = std::move(pallet);
}

void ProductionDispatcherContext::removePallet(const QString id) {
  Pallets.erase(id);
}

SqlQueryValues& ProductionDispatcherContext::pallet(const QString& id) {
  return Pallets[id];
}

SqlQueryValues& ProductionDispatcherContext::order() {
  return Order;
}

SqlQueryValues& ProductionDispatcherContext::issuer() {
  return Issuer;
}

SqlQueryValues& ProductionDispatcherContext::masterKeys() {
  return MasterKeys;
}
