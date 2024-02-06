#ifndef ABSTRACTBOXRELEASESYSTEM_H
#define ABSTRACTBOXRELEASESYSTEM_H

#include "abstract_production_system.h"

class AbstractBoxReleaseSystem : public AbstractProductionSystem {
  Q_OBJECT

 public:
  explicit AbstractBoxReleaseSystem(const QString& name);
  virtual ~AbstractBoxReleaseSystem();

  virtual ReturnStatus request(void) = 0;
  virtual ReturnStatus refund(void) = 0;
  virtual ReturnStatus complete(void) = 0;

 private:
  AbstractBoxReleaseSystem();
  Q_DISABLE_COPY_MOVE(AbstractBoxReleaseSystem)

 signals:
  void boxAssemblyCompleted(void);
  void palletAssemblyCompleted(void);
  void orderAssemblyCompleted(void);
};

#endif // ABSTRACTBOXRELEASESYSTEM_H
