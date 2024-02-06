#ifndef PRODUCTIONLINELAUNCHSYSTEM_H
#define PRODUCTIONLINELAUNCHSYSTEM_H

#include "abstract_launch_system.h"

class ProductionLineLaunchSystem : public AbstractLaunchSystem {
  Q_OBJECT

 public:
  explicit ProductionLineLaunchSystem(const QString& name);
  ~ProductionLineLaunchSystem();

 public:  // AbstractLaunchSystem interface
  virtual ReturnStatus init(void) override;

  virtual ReturnStatus launch(void) override;
  virtual ReturnStatus shutdown(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ProductionLineLaunchSystem)

  ReturnStatus checkProductionLineState(void);
  ReturnStatus loadProductionLine(void);

  bool updateProductionLine(const SqlQueryValues& newValues);
};

#endif  // PRODUCTIONLINELAUNCHSYSTEM_H
