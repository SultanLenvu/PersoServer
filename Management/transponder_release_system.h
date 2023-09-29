#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>

#include "Database/database_controller.h"
#include "Database/postgres_controller.h"

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
    Success,
  };

 private:
  PostgresController* Database;
  QMutex Mutex;

 public:
  explicit TransponderReleaseSystem(QObject* parent);

 public slots:
  void start(ReturnStatus* status);
  void stop(void);
  void applySettings(void);

  void authorize(const QMap<QString, QString>* parameters,
                 ReturnStatus* status);
  void release(const QMap<QString, QString>* parameters,
               QMap<QString, QString>* attributes,
               QMap<QString, QString>* masterKeys,
               ReturnStatus* status);
  void confirmRelease(const QMap<QString, QString>* parameters,
                      QMap<QString, QString>* transponderInfo,
                      ReturnStatus* status);

  void rerelease(const QMap<QString, QString>* parameters,
                 QMap<QString, QString>* attributes,
                 QMap<QString, QString>* masterKeys,
                 ReturnStatus* status);
  void confirmRerelease(const QMap<QString, QString>* parameters,
                        QMap<QString, QString>* transponderInfo,
                        ReturnStatus* status);

  void search(const QMap<QString, QString>* parameters,
              QMap<QString, QString>* attributes,
              QMap<QString, QString>* masterKeys,
              ReturnStatus* status);

 private:
  void createDatabaseController(void);
  void loadSettings(void);

  bool generateTransponderSeed(const QPair<QString, QString>* searchPair,
                               QMap<QString, QString>* attributes,
                               QMap<QString, QString>* masterKeys);
  bool generateTransponderInfo(const QString&, QMap<QString, QString>* info);
  bool checkRerelease(const QMap<QString, QString>& transponderRecord,
                      const QMap<QString, QString>& searchData);

  bool confirmTransponder(const QString& transponderId) const;
  bool confirmBox(const QString& boxId) const;
  bool confirmPallet(const QString& palletId) const;
  bool confirmOrder(const QString& orderId) const;

  bool searchNextTransponderForAssembling(
      QMap<QString, QString>* productionLineRecord) const;

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
  void operationFinished();
};

#endif // TRANSPONDERRELEASESYSTEM_H
