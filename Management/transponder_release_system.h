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
    DatabaseQueryError,
    DatabaseTransactionError,
    DatabaseConnectionError,
    TransponderMissed,
    TransponderNotReleasedEarlier,
    AwaitingConfirmationError,
    IdenticalUcidError,
    ProductionLineMissed,
    ProductionLineNotActive,
    CurrentOrderRunOut,
    CurrentOrderAssembled,
    LogicError,
    Completed,
  };
  Q_ENUM(ReturnStatus);

 private:
  bool LogEnable;
  uint32_t CheckPeriod;
  QTimer* CheckTimer;

  PostgresController* Database;

  QHash<QString, QString> CurrentProductionLine;
  QHash<QString, QString> CurrentTransponder;
  QHash<QString, QString> CurrentBox;
  QHash<QString, QString> CurrentPallet;
  QHash<QString, QString> CurrentOrder;
  QHash<QString, QString> CurrentIssuer;
  QHash<QString, QString> CurrentMasterKeys;
  QHash<QString, QString> SupportData;

  QMutex Mutex;

 public:
  explicit TransponderReleaseSystem(QObject* parent);
  ~TransponderReleaseSystem();

 public slots:
  void on_InstanceThreadStarted_slot(void);

  void start(ReturnStatus* status);
  void stop(void);

  void authorize(const QHash<QString, QString>* parameters,
                 TransponderReleaseSystem::ReturnStatus* status);
  void release(const QHash<QString, QString>* parameters,
               QHash<QString, QString>* seed,
               QHash<QString, QString>* data,
               TransponderReleaseSystem::ReturnStatus* status);
  void confirmRelease(const QHash<QString, QString>* parameters,
                      TransponderReleaseSystem::ReturnStatus* status);
  void rerelease(const QHash<QString, QString>* parameters,
                 QHash<QString, QString>* seed,
                 QHash<QString, QString>* data,
                 TransponderReleaseSystem::ReturnStatus* status);
  void confirmRerelease(const QHash<QString, QString>* parameters,
                        TransponderReleaseSystem::ReturnStatus* status);
  void search(const QHash<QString, QString>* parameters,
              QHash<QString, QString>* data,
              TransponderReleaseSystem::ReturnStatus* status);

  void rollbackProductionLine(const QHash<QString, QString>* parameters,
                              TransponderReleaseSystem::ReturnStatus* status);

 private:
  Q_DISABLE_COPY(TransponderReleaseSystem);
  void createDatabaseController(void);
  void loadSettings(void);
  void sendLog(const QString& log) const;
  void createCheckTimer(void);

  ReturnStatus getCurrentContext(const QHash<QString, QString>* initData);
  void clearCurrentContext(void);

  bool confirmCurrentTransponder(const QString& ucid);
  bool confirmCurrentBox(void);
  bool confirmCurrentPallet(void);
  bool confirmCurrentOrder(void);

  bool searchNextTransponderForCurrentProductionLine(void);
  bool startBoxAssembling(const QString& id);
  bool startPalletAssembling(const QString& id);
  bool linkCurrentProductionLine(const QString& id);
  bool stopCurrentProductionLine(void);

  void generateFirmwareSeed(QHash<QString, QString>* seed) const;
  void generateTransponderData(QHash<QString, QString>* data) const;
  void generateBoxData(QHash<QString, QString>* data) const;
  void generatePalletData(QHash<QString, QString>* data) const;

  QString generateTransponderSerialNumber(const QString& id) const;

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
