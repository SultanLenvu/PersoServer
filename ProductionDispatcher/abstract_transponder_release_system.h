#ifndef ABSTRACTTRANSPONDERELEASESYSTEM_H
#define ABSTRACTTRANSPONDERELEASESYSTEM_H

#include "abstract_production_system.h"

class AbstractTransponderReleaseSystem : public AbstractProductionSystem {
  Q_OBJECT

 public:
  explicit AbstractTransponderReleaseSystem(const QString& name);
  virtual ~AbstractTransponderReleaseSystem();

  virtual ReturnStatus findLastReleased(void) = 0;
  virtual ReturnStatus findNext(void) = 0;
  virtual ReturnStatus release(void) = 0;
  virtual ReturnStatus confirmRelease(const QString& ucid) = 0;
  virtual ReturnStatus rerelease(const QString& key, const QString& value) = 0;
  virtual ReturnStatus confirmRerelease(const QString& key,
                                        const QString& value,
                                        const QString& ucid) = 0;
  virtual ReturnStatus rollback(void) = 0;

 private:
};

#endif  // ABSTRACTTRANSPONDERELEASESYSTEM_H
