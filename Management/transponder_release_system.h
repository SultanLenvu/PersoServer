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
#include "StickerPrinter/isticker_printer.h"

class TransponderReleaseSystem : public QObject {
  Q_OBJECT

 public:
  enum ReturnStatus {
    DatabaseQueryError,
    DatabaseTransactionError,
    DatabaseConnectionError,
    TransponderNotFound,
    TransponderNotReleasedEarlier,
    AwaitingConfirmationError,
    IdenticalUcidError,
    ProductionLineMissed,
    ProductionLineNotActive,
    CurrentOrderRunOut,
    CurrentOrderAssembled,
    ProductionLineRollbackLimitError,
    ProductionLineStopError,
    BoxStickerPrintError,
    PalletStickerPrintError,
    NextTransponderNotFound,
    StartBoxAssemblingError,
    StartPalletAssemblingError,
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
                 ReturnStatus* status);
  void release(const QHash<QString, QString>* parameters,
               QHash<QString, QString>* seed,
               QHash<QString, QString>* data,
               ReturnStatus* status);
  void confirmRelease(const QHash<QString, QString>* parameters,
                      ReturnStatus* status);
  void rerelease(const QHash<QString, QString>* parameters,
                 QHash<QString, QString>* seed,
                 QHash<QString, QString>* data,
                 ReturnStatus* status);
  void confirmRerelease(const QHash<QString, QString>* parameters,
                        ReturnStatus* status);
  void search(const QHash<QString, QString>* parameters,
              QHash<QString, QString>* data,
              ReturnStatus* status);

  void rollbackProductionLine(const QHash<QString, QString>* parameters,
                              ReturnStatus* status);

  void getBoxData(const QHash<QString, QString>* parameters,
                  QHash<QString, QString>* data,
                  ReturnStatus* status);
  void getPalletData(const QHash<QString, QString>* parameters,
                     QHash<QString, QString>* data,
                     ReturnStatus* status);

 private:
  Q_DISABLE_COPY(TransponderReleaseSystem);
  void createDatabaseController(void);
  void loadSettings(void);
  void sendLog(const QString& log) const;
  void createCheckTimer(void);

  ReturnStatus getCurrentContext(const QHash<QString, QString>* initData);
  void clearCurrentContext(void);

  ReturnStatus confirmCurrentTransponder(const QString& ucid);
  ReturnStatus confirmCurrentBox(void);
  ReturnStatus confirmCurrentPallet(void);
  ReturnStatus confirmCurrentOrder(void);

  ReturnStatus searchNextTransponderForCurrentProductionLine(void);
  ReturnStatus startBoxAssembling(const QString& id);
  ReturnStatus startPalletAssembling(const QString& id);
  ReturnStatus linkCurrentProductionLine(const QString& id);
  ReturnStatus stopCurrentProductionLine(void);

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
  void failed(ReturnStatus status);

  void boxAssemblingFinished(const QHash<QString, QString>* data,
                             IStickerPrinter::ReturnStatus* status) const;
  void palletAssemblingFinished(const QHash<QString, QString>* data,
                                IStickerPrinter::ReturnStatus* status) const;
  void orderAssemblingFinished(const QHash<QString, QString>* data,
                               IStickerPrinter::ReturnStatus* status) const;
};

#endif // TRANSPONDERRELEASESYSTEM_H
