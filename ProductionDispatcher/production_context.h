#ifndef PRODUCTION_CONTEXT_H
#define PRODUCTION_CONTEXT_H

#include <QMutex>

#include "Database/sql_query_values.h"
#include "General/types.h"

class AbstractProductionLineContext {
 private:
  QMutex Mutex;

  QHash<QString, StringDictionary> Entities;

 public:
  explicit AbstractProductionLineContext();
  ~AbstractProductionLineContext();

  QString value(const QString& entity, const QString& param) const;
  void addEntity(const QString& name, const SqlQueryValues values);

 private:
  Q_DISABLE_COPY_MOVE(AbstractProductionLineContext)
};

#endif  // PRODUCTION_CONTEXT_H
