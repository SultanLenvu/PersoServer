#include <QMutexLocker>

#include "production_context.h"

ProductionContext::ProductionContext() {}

ProductionContext::~ProductionContext() {}

const QString& ProductionContext::login() const {
  return Login;
}

void ProductionContext::setLogin(QString& login) {
  Login = login;
}

const QString& ProductionContext::password() const {
  return Password;
}

void ProductionContext::setPassword(QString& password) {
  Password = password;
}

bool ProductionContext::isActive() const {
  if (ProductionLine.isEmpty()) {
    return false;
  }

  return ProductionLine.get("active") == "true" ? true : false;
}

bool ProductionContext::isLaunched() const {
  if (ProductionLine.isEmpty()) {
    return false;
  }

  return ProductionLine.get("launched") == "true" ? true : false;
}

SqlQueryValues& ProductionContext::productionLine() {
  return ProductionLine;
}

SqlQueryValues& ProductionContext::transponder() {
  return Transponder;
}

SqlQueryValues& ProductionContext::box() {
  return Box;
}

SqlQueryValues& ProductionContext::pallet() {
  return Pallet;
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
