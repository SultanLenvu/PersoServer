#ifndef ABCTRACTINFOSYSTEM_H
#define ABCTRACTINFOSYSTEM_H

#include "abstract_production_system.h"

class AbstractInfoSystem : public AbstractProductionSystem {
  Q_OBJECT
 public:
  explicit AbstractInfoSystem(const QString& name);
  virtual ~AbstractInfoSystem();

  virtual ReturnStatus updateMainContext(void) = 0;

  virtual ReturnStatus generateProductionLineData(StringDictionary& data) = 0;

  virtual ReturnStatus generateTransponderData(StringDictionary& data) = 0;
  virtual ReturnStatus generateTransponderData(const QString& key,
                                               const QString& value,
                                               StringDictionary& data) = 0;

  virtual ReturnStatus generateFirmwareSeed(StringDictionary& seed) = 0;
  virtual ReturnStatus generateFirmwareSeed(const QString& key,
                                            const QString& value,
                                            StringDictionary& seed) = 0;

  virtual ReturnStatus generateBoxData(StringDictionary& data) = 0;
  virtual ReturnStatus generateBoxData(const QString& id,
                                       StringDictionary& data) = 0;

  virtual ReturnStatus generatePalletData(StringDictionary& data) = 0;
  virtual ReturnStatus generatePalletData(const QString& id,
                                          StringDictionary& data) = 0;

  virtual QString getTransponderBoxId(const QString& key,
                                      const QString& value) = 0;
  virtual QString getTransponderPalletId(const QString& key,
                                         const QString& value) = 0;

 private:
  Q_DISABLE_COPY_MOVE(AbstractInfoSystem)
};

#endif  // ABCTRACTINFOSYSTEM_H
