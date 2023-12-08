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
  virtual ReturnStatus init(const StringDictionary& param) const override;
  virtual ReturnStatus launch(const StringDictionary& param) const override;
  virtual ReturnStatus shutdown(const StringDictionary& param) const override;
  virtual bool isLaunched(const StringDictionary& param) const override;

 private:
  Q_DISABLE_COPY_MOVE(ProductionLineLaunchSystem)
  void loadSettings(void);
  void sendLog(const QString& log) const;

  ReturnStatus attachWithFreeBox(const QString& id) const;
  ReturnStatus detachFromBox(const QString& id, const QString& boxId) const;
};

#endif  // PRODUCTIONLINELAUNCHSYSTEM_H
