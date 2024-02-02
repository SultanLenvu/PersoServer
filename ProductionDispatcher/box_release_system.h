#ifndef BOXRELEASESYSTEM_H
#define BOXRELEASESYSTEM_H

#include "abstract_box_release_system.h"

class BoxReleaseSystem : public AbstractBoxReleaseSystem
{
  Q_OBJECT

 public:
  explicit BoxReleaseSystem(const QString& name,
                            std::shared_ptr<AbstractSqlDatabase> db);
  ~BoxReleaseSystem();

 public:  // AbstractBoxReleaseSystem interface
  virtual void setContext(
      std::shared_ptr<ProductionLineContext> context) override;

  virtual ReturnStatus request(void) override;
  virtual ReturnStatus refund(void) override;
  virtual ReturnStatus complete(void) override;
  virtual void clearContext(void) override;

 private:
  Q_DISABLE_COPY_MOVE(BoxReleaseSystem)
  void loadSettings(void);
  void sendLog(const QString& log);

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

  bool updateProductionLine(const SqlQueryValues& newValues);
  bool updateTransponder(const SqlQueryValues& newValues);
  bool updateBox(const SqlQueryValues& newValues);
  bool updatePallet(const SqlQueryValues& newValues);
  bool updateOrder(const SqlQueryValues& newValues);
};

#endif // BOXRELEASESYSTEM_H
