#include "abstract_production_dispatcher.h"
#include "Management/global_context.h"

AbstractProductionDispatcher::AbstractProductionDispatcher(const QString& name)
    : QObject{nullptr} {
  setObjectName(name);

  // Выбрасываем указатель на глобальный контекст
  GlobalContext::instance()->registerObject(
      std::make_shared<AbstractProductionDispatcher>(this));
}
