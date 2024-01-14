#include "global_environment.h"

GlobalEnvironment::~GlobalEnvironment() {}

GlobalEnvironment* GlobalEnvironment::instance() {
  static GlobalEnvironment context;

  return &context;
}

void GlobalEnvironment::registerObject(QObject* obj) {
  GlobalObjects[obj->objectName()] = obj;

  connect(obj, &QObject::destroyed, this,
          &GlobalEnvironment::onRegisteredObjectDeleted);
}

QObject* GlobalEnvironment::getObject(const QString& name) {
  if (!GlobalObjects.contains(name)) {
    return nullptr;
  }

  return GlobalObjects.value(name);
}

GlobalEnvironment::GlobalEnvironment() {}

void GlobalEnvironment::onRegisteredObjectDeleted() {
  GlobalObjects.remove(sender()->objectName());
}
