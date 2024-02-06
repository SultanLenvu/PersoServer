#ifndef ABSTRACTPRODUCTIONSYSTEM_H
#define ABSTRACTPRODUCTIONSYSTEM_H

#include <QObject>

#include "abstract_sql_database.h"
#include "production_context.h"
#include "production_line_context.h"
#include "psobject.h"

class AbstractProductionSystem : public PSObject {
  Q_OBJECT

 protected:
  std::shared_ptr<AbstractSqlDatabase> Database;

  std::shared_ptr<ProductionContext> MainContext;
  std::shared_ptr<ProductionLineContext> SubContext;

 public:
  explicit AbstractProductionSystem(const QString& name);
  virtual ~AbstractProductionSystem();

 public:
  bool isValid(void) const;

 public:
  void setDatabase(std::shared_ptr<AbstractSqlDatabase> db);
  void setMainContext(std::shared_ptr<ProductionContext> mc);
  void setSubContext(std::shared_ptr<ProductionLineContext> sc);
};

#endif // ABSTRACTPRODUCTIONSYSTEM_H
