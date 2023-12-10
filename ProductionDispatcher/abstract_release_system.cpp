#include "abstract_release_system.h"

AbstractReleaseSystem::AbstractReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : QObject(nullptr) {
  setObjectName(name);
  Database = db;
}

AbstractReleaseSystem::~AbstractReleaseSystem() {}

AbstractReleaseSystem::AbstractReleaseSystem()
    : QObject{nullptr} {}
