#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSharedPointer>
#include <QTimer>

#include "Database/database_controller.h"
#include "Database/postgres_controller.h"
#include "Log/log_system.h"

class TransponderReleaseSystem : public QObject {
  Q_OBJECT

 public:
  enum ReturnStatus {
    Undefined,
    Failed,
    DatabaseConnectionError,
    TransactionError,
    ProductionLineMissed,
    ProductionLineNotActive,
    CurrentOrderRunOut,
    CurrentOrderAssembled,
    Completed,
  };
  Q_ENUM(ReturnStatus);

 private:
  bool LogEnable;
  uint32_t CheckPeriod;

  PostgresController* Database;
  QMutex Mutex;

  QTimer* CheckTimer;

 public:
  explicit TransponderReleaseSystem(QObject* parent);
  ~TransponderReleaseSystem();

 public slots:
  void on_InstanceThreadStarted_slot(void);

  void start(ReturnStatus* status);
  void stop(void);

  void authorize(const QHash<QString, QString>* parameters,
                 ReturnStatus* status);
  void release(const QHash<QString, QString>* parameters,
               QHash<QString, QString>* attributes,
               QHash<QString, QString>* masterKeys,
               ReturnStatus* status);
  void confirmRelease(const QHash<QString, QString>* parameters,
                      QHash<QString, QString>* transponderData,
                      ReturnStatus* status);

  void rerelease(const QHash<QString, QString>* parameters,
                 QHash<QString, QString>* attributes,
                 QHash<QString, QString>* masterKeys,
                 ReturnStatus* status);
  void confirmRerelease(const QHash<QString, QString>* parameters,
                        QHash<QString, QString>* transponderData,
                        ReturnStatus* status);

  void search(const QHash<QString, QString>* parameters,
              QHash<QString, QString>* attributes,
              QHash<QString, QString>* masterKeys,
              ReturnStatus* status);

 private:
  Q_DISABLE_COPY(TransponderReleaseSystem);
  void createDatabaseController(void);
  void loadSettings(void);
  void sendLog(const QString& log) const;
  void createCheckTimer(void);

  bool checkConfirmRerelease(const QHash<QString, QString>& transponderRecord,
                             const QHash<QString, QString>& searchData);

  bool confirmTransponder(const QString& transponderId,
                          const QString& ucid) const;
  bool confirmBox(const QString& boxId) const;
  bool confirmPallet(const QString& palletId) const;
  bool confirmOrder(const QString& orderId) const;

  bool searchNextTransponderForAssembling(
      QHash<QString, QString>* productionLineRecord) const;

  bool getTransponderSeed(const QPair<QString, QString>* searchPair,
                          QHash<QString, QString>* attributes,
                          QHash<QString, QString>* masterKeys) const;
  bool getTransponderData(const QString&, QHash<QString, QString>* data) const;
  bool getBoxData(const QString& id, QHash<QString, QString>* data) const;
  bool getPalletData(const QString& id, QHash<QString, QString>* data) const;
  bool getOrderData(const QString& id, QHash<QString, QString>* data) const;

 private slots:
  void on_CheckTimerTemeout(void);

 signals:
  void logging(const QString& log) const;

  void operationFinished();
  void failed(TransponderReleaseSystem::ReturnStatus status);

  void boxAssemblingFinished(
      const QSharedPointer<QHash<QString, QString> > data) const;
  void palletAssemblingFinished(
      const QSharedPointer<QHash<QString, QString> > data) const;
  void orderAssemblingFinished(
      const QSharedPointer<QHash<QString, QString> > data) const;
};

#endif // TRANSPONDERRELEASESYSTEM_H
