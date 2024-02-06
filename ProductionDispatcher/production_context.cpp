#include "production_context.h"

ProductionContext::ProductionContext()
    : AbstractContext() {}

ProductionContext::~ProductionContext() {}

void ProductionContext::clear() {
  PalletStorage.clear();
  Pallet->clear();
  Order.clear();
  Issuer.clear();
  MasterKeys.clear();
}

void ProductionContext::stash() {
  Stash = std::unique_ptr<ProductionContext>(
      new ProductionContext());

  Stash->PalletStorage = PalletStorage;
  Stash->Pallet = Pallet;
  Stash->Order = Order;
  Stash->Issuer = Issuer;
  Stash->MasterKeys = MasterKeys;
}

void ProductionContext::applyStash() {
  if (!Stash) {
    return;
  }

  PalletStorage = Stash->PalletStorage;
  Pallet = Stash->Pallet;
  Order = Stash->Order;
  Issuer = Stash->Issuer;
  MasterKeys = Stash->MasterKeys;
}

bool ProductionContext::isValid() {
  if (!Pallet || Pallet->isEmpty() || Order.isEmpty() || Issuer.isEmpty() ||
      MasterKeys.isEmpty()) {
    return false;
  }

  return true;
}

void ProductionContext::removePallet(const QString id) {
  PalletStorage.erase(id);
}

SqlQueryValues& ProductionContext::pallet(const QString& id) {
  if (PalletStorage.count(id) > 0) {
    if (Pallet != PalletStorage[id]) {
      Pallet = PalletStorage[id];
    }

    return *Pallet;
  }

  Pallet = std::shared_ptr<SqlQueryValues>(new SqlQueryValues());
  PalletStorage[id] = Pallet;

  return *Pallet;
}

SqlQueryValues& ProductionContext::order() {
  return Order;
}

SqlQueryValues& ProductionContext::issuer() {
  return Issuer;
}

SqlQueryValues& ProductionContext::masterKeys() {
  return MasterKeys;
}
