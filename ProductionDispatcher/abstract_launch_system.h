#ifndef AbstractLaunchSystem_H
#define AbstractLaunchSystem_H

#include "abstract_production_system.h"

class AbstractLaunchSystem : public AbstractProductionSystem {
  Q_OBJECT

 public:
  explicit AbstractLaunchSystem(const QString& name);
  virtual ~AbstractLaunchSystem();

  virtual ReturnStatus init(void) = 0;

  virtual ReturnStatus launch(void) = 0;
  virtual ReturnStatus shutdown(void) = 0;

 private:
  Q_DISABLE_COPY_MOVE(AbstractLaunchSystem)
};

#endif  // AbstractLaunchSystem_H
