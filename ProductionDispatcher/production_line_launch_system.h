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

  virtual ReturnStatus requestBox(void) override;
  virtual ReturnStatus refundBox(void) override;
  virtual ReturnStatus completeBox(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ProductionLineLaunchSystem)
  void loadSettings(void);
  void sendLog(const QString& log);

  ReturnStatus checkProductionLineState(void);

  ReturnStatus refundBoxSubprocess(void);

  ReturnStatus findOrderInProcess(void);
  ReturnStatus findBox(void);

  bool attachBox(void);
  bool detachBox(void);

  ReturnStatus startBoxAssembly(void);
  ReturnStatus startPalletAssembly(void);

  bool stopBoxAssembly(void);
  bool stopPalletAssembly(void);

  ReturnStatus completePallet(void);
  ReturnStatus completeOrder(void);

  ReturnStatus loadBoxContext(void);
  ReturnStatus loadProductionLine(void);
  ReturnStatus loadOrderInProcess(void);

  bool updateProductionLine(const SqlQueryValues& newValues);
  bool updateTransponder(const SqlQueryValues& newValues);
  bool updateBox(const SqlQueryValues& newValues);
  bool updatePallet(const SqlQueryValues& newValues);
  bool updateOrder(const SqlQueryValues& newValues);
};

#endif  // PRODUCTIONLINELAUNCHSYSTEM_H
