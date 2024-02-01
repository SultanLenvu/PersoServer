#ifndef AbstractLaunchSystem_H
#define AbstractLaunchSystem_H

#include <QObject>

#include "abstract_sql_database.h"
#include "production_line_context.h"
#include "types.h"

class AbstractLaunchSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionLineContext> Context;
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractLaunchSystem(const QString& name,
                                std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractLaunchSystem();

  virtual void setContext(std::shared_ptr<ProductionLineContext> context) = 0;
  virtual ReturnStatus init(void) = 0;

  virtual ReturnStatus launch(void) = 0;
  virtual ReturnStatus shutdown(void) = 0;

 private:
  AbstractLaunchSystem();
  Q_DISABLE_COPY_MOVE(AbstractLaunchSystem)

 signals:
  void logging(const QString& log);
};

#endif  // AbstractLaunchSystem_H
