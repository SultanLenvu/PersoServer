#include "abstract_launch_system.h"

AbstractLaunchSystem::AbstractLaunchSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;
}

AbstractLaunchSystem::~AbstractLaunchSystem() {}

AbstractLaunchSystem::AbstractLaunchSystem() : QObject{nullptr} {}
