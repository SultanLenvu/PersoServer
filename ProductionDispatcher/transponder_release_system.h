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
#include "ProductionDispatcher/abstract_release_system.h"

class TransponderReleaseSystem : public AbstractReleaseSystem {
  Q_OBJECT

 private:
  bool ContextReady;

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
  virtual void setContext(const ProductionContext& context) override;
  virtual ReturnStatus release(void) override;
  virtual ReturnStatus confirmRelease(const StringDictionary& param) override;
  virtual ReturnStatus rerelease(const StringDictionary& param) override;
  virtual ReturnStatus confirmRerelease(const StringDictionary& param) override;
  virtual ReturnStatus rollback(void) override;

 private:
  Q_DISABLE_COPY_MOVE(TransponderReleaseSystem)
  void loadSettings(void);
  void sendLog(const QString& log) const;

  bool confirmCurrentTransponder(const QString& ucid);
  bool completeCurrentBoxAssembly(void);
  bool completeCurrentPalletAssembly(void);
  bool completeCurrentOrderAssembly(void);

  ReturnStatus searchNextTransponder(void);
  ReturnStatus searchNextBox(void);

  bool startCurrentBoxAssembly(void);
  bool startCurrentPalletAssembly(void);

  bool switchCurrentTransponder(const QString& id);
  bool switchCurrentBox(const QString& id);
  bool switchCurrentPallet(const QString& id);

  bool updateCurrentProductionLine(const SqlQueryValues& newValues);
  bool updateCurrentTransponder(const SqlQueryValues& newValues);
  bool updateCurrentBox(const SqlQueryValues& newValues);
  bool updateCurrentPallet(const SqlQueryValues& newValues);
  bool updateCurrentOrder(const SqlQueryValues& newValues);

 signals:
  void boxAssemblyCompleted(const std::shared_ptr<QString> id);
  void palletAssemblyCompleted(const std::shared_ptr<QString> id);
  void orderAssemblyCompleted(const std::shared_ptr<QString> id);
};

#endif  // TRANSPONDERRELEASESYSTEM_H
