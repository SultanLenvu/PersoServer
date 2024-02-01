#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

#include "abstract_transponder_release_system.h"
#include "abstract_sql_database.h"

class TransponderReleaseSystem : public AbstractTransponderReleaseSystem {
  Q_OBJECT

 public:
  explicit TransponderReleaseSystem(const QString& name,
                                    std::shared_ptr<AbstractSqlDatabase> db);
  ~TransponderReleaseSystem();

  // AbstractTransponderReleaseSystem interface
 public:
  virtual void setContext(
      std::shared_ptr<ProductionLineContext> context) override;

  virtual ReturnStatus findLastReleased(void) override;
  virtual ReturnStatus findNext(void) override;
  virtual ReturnStatus release(void) override;
  virtual ReturnStatus confirmRelease(const QString& ucid) override;
  virtual ReturnStatus rerelease(const QString& key,
                                 const QString& value) override;
  virtual ReturnStatus confirmRerelease(const QString& key,
                                        const QString& value,
                                        const QString& ucid) override;
  virtual ReturnStatus rollback(void) override;

 private:
  Q_DISABLE_COPY_MOVE(TransponderReleaseSystem)
  void loadSettings(void);
  void sendLog(const QString& log) const;

  ReturnStatus checkContext(void);

  bool confirmTransponder(const QString& ucid);
  bool attachTransponder(void);

  bool switchTransponder(const QString& id);
  bool switchCurrentBox(const QString& id);
  bool switchCurrentPallet(const QString& id);

  bool updateProductionLine(const SqlQueryValues& newValues);
  bool updateTransponder(const SqlQueryValues& newValues);
  bool updateBox(const SqlQueryValues& newValues);
};

#endif  // TRANSPONDERRELEASESYSTEM_H
