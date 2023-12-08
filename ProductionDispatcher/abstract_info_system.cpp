#include "abstract_info_system.h"

AbstractInfoSystem::AbstractInfoSystem(
    const QString& name,
    const std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;
}

AbstractInfoSystem::~AbstractInfoSystem() {}

AbstractInfoSystem::AbstractInfoSystem() : QObject{nullptr} {}
