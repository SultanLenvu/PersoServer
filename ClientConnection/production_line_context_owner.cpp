#include "production_line_context_owner.h"

ProductionLineContextOwner::ProductionLineContextOwner(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);
}

ProductionLineContextOwner::~ProductionLineContextOwner() {}

std::shared_ptr<ProductionLineContext> ProductionLineContextOwner::context() const {
  return Context;
}

void ProductionLineContextOwner::setContext(
    std::shared_ptr<ProductionLineContext> context) {
  Context = context;
}
