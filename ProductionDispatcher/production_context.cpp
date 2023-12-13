#include <QMutexLocker>

#include "production_context.h"

AbstractProductionLineContext::AbstractProductionLineContext() {}

AbstractProductionLineContext::~AbstractProductionLineContext() {}

QString AbstractProductionLineContext::value(const QString& entity,
                                         const QString& param) const {
  if (!Entities.contains(entity)) {
    return QString();
  }

  return Entities.value(entity).value(param);
}

void AbstractProductionLineContext::addEntity(const QString& name,
                                          const SqlQueryValues values) {}
