#include "production_dispatcher_context.h"

ProductionDispatcherContext::ProductionDispatcherContext() : AbstractContext() {
  Pallets[""] = SqlQueryValues();
}

ProductionDispatcherContext::~ProductionDispatcherContext() {}

void ProductionDispatcherContext::stash() {
  Stash = std::unique_ptr<ProductionDispatcherContext>(
      new ProductionDispatcherContext());

  Stash->Pallets = Pallets;
  Stash->Order = Order;
  Stash->Issuer = Issuer;
  Stash->MasterKeys = MasterKeys;
}

void ProductionDispatcherContext::applyStash() {
  if (!Stash) {
    return;
  }

  Pallets = Stash->Pallets;
  Order = Stash->Order;
  Issuer = Stash->Issuer;
  MasterKeys = Stash->MasterKeys;
}

void ProductionDispatcherContext::addPallet(const SqlQueryValues& pallet) {
  if (pallet.get("id").isEmpty()) {
    return;
  }

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
