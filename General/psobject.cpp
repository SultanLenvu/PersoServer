#include "psobject.h"
#include "global_environment.h"
#include "log_system.h"

PSObject::PSObject(const QString& name) : QObject{nullptr} {
  setObjectName(name);

  connectDependencies();
}

PSObject::~PSObject() {}

void PSObject::applySettings() {
  sendLog("Применение новых настроек.");

  loadSettings();
}

void PSObject::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
}

PSObject::PSObject() {}

void PSObject::connectDependencies() {
  LogSystem* ls = dynamic_cast<LogSystem*>(
      GlobalEnvironment::instance()->getObject("LogSystem"));
  assert(ls);

  connect(this, &PSObject::logging, ls, &LogSystem::generate);
}

void PSObject::loadSettings() {}
