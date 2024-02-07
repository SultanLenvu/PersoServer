#include <QMutexLocker>

#include "production_line_context.h"

ProductionLineContext::ProductionLineContext() : AbstractContext() {}

ProductionLineContext::~ProductionLineContext() {}

void ProductionLineContext::clear() {
  ProductionLine.clear();
  Box.clear();
  Transponder.clear();

  Login.clear();
  Password.clear();
}

void ProductionLineContext::stash() {
  Stash = std::unique_ptr<ProductionLineContext>(new ProductionLineContext());

  Stash->setLogin(Login);
  Stash->setPassword(Password);

  Stash->ProductionLine = ProductionLine;
  Stash->Transponder = Transponder;
  Stash->Box = Box;
}

void ProductionLineContext::applyStash() {
  if (!Stash) {
    return;
  }

  Login = Stash->Login;
  Password = Stash->Password;

  ProductionLine = Stash->ProductionLine;
  Transponder = Stash->Transponder;
  Box = Stash->Box;
}

const QString& ProductionLineContext::login() const {
  return Login;
}

void ProductionLineContext::setLogin(const QString& login) {
  Login = login;
}

const QString& ProductionLineContext::password() const {
  return Password;
}

void ProductionLineContext::setPassword(const QString& password) {
  Password = password;
}

bool ProductionLineContext::isActive() const {
  if (ProductionLine.isEmpty()) {
    return false;
  }

  return ProductionLine.get("active") == "true" ? true : false;
}

bool ProductionLineContext::isLaunched() const {
  if (ProductionLine.isEmpty()) {
    return false;
  }

  return ProductionLine.get("launched") == "true" ? true : false;
}

bool ProductionLineContext::isAuthorized() const {
  if (Password.isEmpty() || Login.isEmpty()) {
    return false;
  }

  return true;
}

bool ProductionLineContext::isInProcess() const {
  if (!isLaunched() || Box.isEmpty()) {
    return false;
  }

  if ((ProductionLine.get("in_process") == "false") &&
      (ProductionLine.get("box_id") == "0") &&
      (ProductionLine.get("transponder_id") == "0")) {
    return false;
  }

  return true;
}

SqlQueryValues& ProductionLineContext::productionLine() {
  return ProductionLine;
}

SqlQueryValues& ProductionLineContext::transponder() {
  return Transponder;
}

SqlQueryValues& ProductionLineContext::box() {
  return Box;
}
