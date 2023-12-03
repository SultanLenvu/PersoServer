#include "global_context.h"

#include "Log/log_system.h"
#include "ProductionDispatcher/authorization_system.h"
#include "ProductionDispatcher/general_production_dispatcher.h"
#include "ProductionDispatcher/info_system.h"
#include "ProductionDispatcher/transponder_release_system.h"

GlobalContext::~GlobalContext() {}

GlobalContext* GlobalContext::instance() {
  static GlobalContext context(nullptr);

  return &context;
}

void GlobalContext::registerObject(std::shared_ptr<QObject> obj) {
  GlobalObjects.insert(obj->objectName(), obj);
}

const std::shared_ptr<QObject> GlobalContext::getObject(const QString& name) {
  if (!GlobalObjects.contains(name)) {
    return nullptr;
  }

  return GlobalObjects.value(name);
}

AbstractProductionDispatcher* GlobalContext::createProductionDispatcher(
    QObject* parent) {
  AbstractProductionDispatcher* pd = new GeneralProductionDispatcher(parent);
  connect(pd, &AbstractProductionDispatcher::logging, LogSystem::instance(),
          &LogSystem::generate);

  return pd;
}

AbstractAuthorizationSystem* GlobalContext::createAuthorizationSystem(
    QObject* parent) {
  AbstractAuthorizationSystem* as = new AuthorizationSystem(parent);
  connect(as, &AbstractAuthorizationSystem::logging, LogSystem::instance(),
          &LogSystem::generate);

  return as;
}

AbstractInfoSystem* GlobalContext::createInfoSystem(QObject* parent) {
  AbstractInfoSystem* is = new InfoSystem(parent);
  connect(is, &AbstractInfoSystem::logging, LogSystem::instance(),
          &LogSystem::generate);

  return is;
}

AbstractTransponderReleaseSystem* GlobalContext::createTransponderReleaseSystem(
    QObject* parent) {
  AbstractTransponderReleaseSystem* trs = new TransponderReleaseSystem(parent);
  connect(trs, &AbstractTransponderReleaseSystem::logging,
          LogSystem::instance(), &LogSystem::generate);

  return trs;
}

GlobalContext::GlobalContext() {}

GlobalContext::GlobalContext(QObject* parent) : QObject{parent} {}
