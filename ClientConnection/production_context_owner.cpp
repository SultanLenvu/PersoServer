#include "production_context_owner.h"

ProductionContextOwner::ProductionContextOwner(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);
}

ProductionContextOwner::~ProductionContextOwner() {}

std::shared_ptr<ProductionLineContext> ProductionContextOwner::context() const {
  return Context;
}

void ProductionContextOwner::setContext(
    std::shared_ptr<ProductionLineContext> context) {
  Context = context;
}
