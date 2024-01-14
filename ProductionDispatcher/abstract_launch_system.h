#ifndef AbstractLaunchSystem_H
#define AbstractLaunchSystem_H

#include <QObject>

#include "abstract_sql_database.h"
#include "production_context.h"
#include "types.h"

class AbstractLaunchSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionContext> Context;
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractLaunchSystem(const QString& name,
                                std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractLaunchSystem();

  virtual void setContext(std::shared_ptr<ProductionContext> context) = 0;
  virtual ReturnStatus init(void) = 0;

  virtual ReturnStatus launch(void) = 0;
  virtual ReturnStatus shutdown(void) = 0;
  virtual bool isLaunched(void) = 0;

  virtual ReturnStatus findBox(void) = 0;
  virtual ReturnStatus refundBox(void) = 0;
  virtual ReturnStatus completeBox(void) = 0;

 private:
  AbstractLaunchSystem();
  Q_DISABLE_COPY_MOVE(AbstractLaunchSystem)

 signals:
  void logging(const QString& log);
  void boxAssemblyCompleted(const std::shared_ptr<QString> id);
  void palletAssemblyCompleted(const std::shared_ptr<QString> id);
  void orderAssemblyCompleted(const std::shared_ptr<QString> id);
};

#endif  // AbstractLaunchSystem_H
