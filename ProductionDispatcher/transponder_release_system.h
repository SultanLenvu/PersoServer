#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

#include "Database/abstract_sql_database.h"
#include "Database/sql_query_values.h"
#include "ProductionDispatcher/abstract_transponder_release_system.h"

class TransponderReleaseSystem : public AbstractReleaseSystem {
  Q_OBJECT

 private:
  std::shared_ptr<SqlQueryValues> CurrentProductionLine;
  std::shared_ptr<SqlQueryValues> CurrentBox;
  std::shared_ptr<SqlQueryValues> CurrentTransponder;
  std::shared_ptr<SqlQueryValues> CurrentPallet;
  std::shared_ptr<SqlQueryValues> CurrentOrder;
  std::shared_ptr<SqlQueryValues> CurrentIssuer;
  std::shared_ptr<SqlQueryValues> CurrentMasterKeys;

 public:
  explicit TransponderReleaseSystem(const QString& name,
                                    std::shared_ptr<AbstractSqlDatabase> db);
  ~TransponderReleaseSystem();

  // AbstractReleaseSystem interface
 public:
  virtual ReturnStatus release(const ProductionContext& context,
                               const StringDictionary& param) override;
  virtual ReturnStatus confirmRelease(const ProductionContext& context,
                                      const StringDictionary& param) override;
  virtual ReturnStatus rerelease(const ProductionContext& context,
                                 const StringDictionary& param) override;
  virtual ReturnStatus confirmRerelease(const ProductionContext& context,
                                        const StringDictionary& param) override;
  virtual ReturnStatus rollback(const ProductionContext& context,
                                const StringDictionary& param) override;

 private:
  Q_DISABLE_COPY_MOVE(TransponderReleaseSystem);
  void loadSettings(void);
  void sendLog(const QString& log) const;

  ReturnStatus confirmCurrentTransponder(const QString& ucid);
  ReturnStatus confirmCurrentBox(void);
  ReturnStatus confirmCurrentPallet(void);
  ReturnStatus confirmCurrentOrder(void);

  ReturnStatus searchNextTransponderForCurrentProductionLine(void);
  ReturnStatus startBoxAssembling(const QString& id);
  ReturnStatus startPalletAssembling(const QString& id);

  void generateFirmwareSeed(StringDictionary& seed) const;

 signals:
  void boxAssemblingFinished(const QString& id);
  void palletAssemblingFinished(const QString& id);
  void orderAssemblingFinished(const QString& id);
};

#endif  // TRANSPONDERRELEASESYSTEM_H
