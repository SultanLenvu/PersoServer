#include "abstract_production_system.h"

AbstractProductionSystem::AbstractProductionSystem(const QString& name)
    : PSObject{name} {}

AbstractProductionSystem::~AbstractProductionSystem() {}

bool AbstractProductionSystem::isValid() const {
  if (!Database || !MainContext || !SubContext) {
    return false;
  }

  return true;
}

void AbstractProductionSystem::setDatabase(
    std::shared_ptr<AbstractSqlDatabase> db) {
  Database = db;
}

void AbstractProductionSystem::setMainContext(
    std::shared_ptr<ProductionContext> mc) {
  MainContext = mc;
}

void AbstractProductionSystem::setSubContext(
    std::shared_ptr<ProductionLineContext> sc) {
  SubContext = sc;
}
