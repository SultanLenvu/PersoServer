#include "global_environment.h"

GlobalEnvironment::~GlobalEnvironment() {}

GlobalEnvironment* GlobalEnvironment::instance() {
  static GlobalEnvironment context;

  return &context;
}

void GlobalEnvironment::registerObject(QObject* obj) {
  GlobalObjects.insert(obj->objectName(), obj);
}

QObject* GlobalEnvironment::getObject(const QString& name) {
  if (!GlobalObjects.contains(name)) {
    return nullptr;
  }

  return GlobalObjects.value(name);
}

GlobalEnvironment::GlobalEnvironment() {}
