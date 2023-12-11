#include "abstract_production_dispatcher.h"
#include "Management/global_environment.h"

AbstractProductionDispatcher::AbstractProductionDispatcher(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);
}
