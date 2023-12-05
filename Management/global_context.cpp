#include "global_context.h"

GlobalContext::~GlobalContext() {}

GlobalContext* GlobalContext::instance() {
  static GlobalContext context;

  return &context;
}

void GlobalContext::registerObject(const QObject* obj) {
  GlobalObjects.insert(obj->objectName(), obj);
}

const QObject* GlobalContext::getObject(const QString& name) {
  if (!GlobalObjects.contains(name)) {
    return nullptr;
  }

  return GlobalObjects.value(name);
}

GlobalContext::GlobalContext() {}
