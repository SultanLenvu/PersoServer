#ifndef ABCTRACTINFOSYSTEM_H
#define ABCTRACTINFOSYSTEM_H

#include <QObject>

#include "Database/abstract_sql_database.h"
#include "General/types.h"

class AbstractInfoSystem : public QObject {
  Q_OBJECT
 protected:
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractInfoSystem(const QString& name,
                              const std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractInfoSystem() = 0;

  virtual void setContext(const ProductionContext& context) = 0;
  virtual ReturnStatus generateProductionContext(
      const QString&,
      ProductionContext& context) = 0;
  virtual ReturnStatus generateTransponderData(StringDictionary& param,
                                               StringDictionary& data) = 0;
  virtual ReturnStatus generateTransponderData(StringDictionary& data) = 0;
  virtual ReturnStatus generateFirmwareSeed(StringDictionary& param,
                                            StringDictionary& seed) = 0;
  virtual ReturnStatus generateFirmwareSeed(StringDictionary& seed) = 0;
  virtual ReturnStatus generateBoxData(StringDictionary& data) = 0;
  virtual ReturnStatus generatePalletData(StringDictionary& data) = 0;
  virtual void reset(void) = 0;

 private:
  AbstractInfoSystem();
  Q_DISABLE_COPY_MOVE(AbstractInfoSystem)
};

#endif  // ABCTRACTINFOSYSTEM_H
