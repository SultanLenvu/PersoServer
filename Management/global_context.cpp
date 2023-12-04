#include "global_context.h"

GlobalContext::~GlobalContext() {}

GlobalContext* GlobalContext::instance() {
  static GlobalContext context;

  return &context;
}

void GlobalContext::registerObject(std::shared_ptr<QObject> obj) {
  GlobalObjects.insert(obj->objectName(), obj);
}

const std::shared_ptr<QObject> GlobalContext::getObject(const QString& name) {
  if (!GlobalObjects.contains(name)) {
    return std::shared_ptr<QObject>();
  }

  return GlobalObjects.value(name);
}

GlobalContext::GlobalContext() {}
