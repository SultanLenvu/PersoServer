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
  virtual ~AbstractInfoSystem();

  virtual void setContext(const ProductionLineContext& context) = 0;
  virtual ReturnStatus loadProductionLineContext(
      const QString&,
      ProductionLineContext& context) = 0;
  virtual QString getTransponderBoxId(const QString& key,
                                      const QString& value) = 0;
  virtual QString getTransponderPalletId(const QString& key,
                                         const QString& value) = 0;
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
  virtual void reset(void) = 0;

 private:
  AbstractInfoSystem();
  Q_DISABLE_COPY_MOVE(AbstractInfoSystem)
};

#endif  // ABCTRACTINFOSYSTEM_H
