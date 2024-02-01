#ifndef PRODUCTIONLINELAUNCHSYSTEM_H
#define PRODUCTIONLINELAUNCHSYSTEM_H

#include "abstract_launch_system.h"

class ProductionLineLaunchSystem : public AbstractLaunchSystem {
  Q_OBJECT

 public:
  explicit ProductionLineLaunchSystem(const QString& name,
                                      std::shared_ptr<AbstractSqlDatabase> db);
  ~ProductionLineLaunchSystem();

  // AbstractLaunchSystem interface
 public:
  virtual void setContext(std::shared_ptr<ProductionLineContext> context) override;

  virtual ReturnStatus init(void) override;
  virtual ReturnStatus launch(void) override;
  virtual ReturnStatus shutdown(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ProductionLineLaunchSystem)
  void loadSettings(void);
  void sendLog(const QString& log);

  ReturnStatus checkProductionLineState(void);

  ReturnStatus loadProductionLine(void);
  ReturnStatus loadOrderInProcess(void);

  bool updateProductionLine(const SqlQueryValues& newValues);
};

#endif  // PRODUCTIONLINELAUNCHSYSTEM_H
