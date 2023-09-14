#ifndef ORDERCREATIONSYSTEM_H
#define ORDERCREATIONSYSTEM_H

#include <QApplication>
#include <QDate>
#include <QObject>

#include "Database/database_controller.h"
#include "Database/database_table_model.h"
#include "Database/postgres_controller.h"
#include "firmware_generation_system.h"
#include "transponder_data_model.h"
#include "transponder_release_system.h"

class AdministrationSystem : public QObject {
  Q_OBJECT
 public:
  enum ExecutionStatus {
    NotExecuted,
    DatabaseConnectionError,
    DatabaseQueryError,
    LogicError,
    ReleaserError,
    UnknowError,
    CompletedSuccessfully
  };

 private:
  PostgresController* Database;
  TransponderReleaseSystem* Releaser;
  FirmwareGenerationSystem* Generator;

 public:
  explicit AdministrationSystem(QObject* parent);
  void applySettings(void);

 public slots:
  void clearDatabaseTable(const QString& tableName);
  void getDatabaseTable(const QString& tableName, DatabaseTableModel* buffer);
  void getCustomResponse(const QString& req, DatabaseTableModel* buffer);

  void createNewOrder(const QMap<QString, QString>* orderParameters);
  void startOrderAssembling(const QString& orderId);
  void stopOrderAssembling(const QString& orderId);
  void deleteLastOrder(void);

  void createNewProductionLine(
      const QMap<QString, QString>* productionLineParameters);
  void allocateInactiveProductionLines(const QString& orderId);
  void linkProductionLineWithBox(const QMap<QString, QString>* linkParameters);
  void shutdownAllProductionLines(void);
  void deleteLastProductionLine(void);

  void releaseTransponder(TransponderSeedModel* model);
  void confirmReleaseTransponder(TransponderSeedModel* model);
  void rereleaseTransponder(TransponderSeedModel* model);
  void confirmRereleaseTransponder(TransponderSeedModel* model);
  void searchTransponder(TransponderSeedModel* model);
  void refundTransponder(TransponderSeedModel* model);

  void initIssuerTable(void);
  void initTransportMasterKeysTable(void);
  void linkIssuerWithMasterKeys(const QMap<QString, QString>* linkParameters);

 private:
  void createDatabaseController(void);
  void loadSettings(void);

  bool initOperation(void);
  void processingOperationResult(const QString& log,
                                 const ExecutionStatus status);

  bool addOrder(const QMap<QString, QString>* orderParameters) const;
  bool addPallets(const QMap<QString, QString>* orderParameters) const;
  bool addBoxes(const QMap<QString, QString>* orderParameters) const;
  bool addTransponders(const QMap<QString, QString>* orderParameters) const;
  bool addProductionLine(
      const QMap<QString, QString>* productionLineParameters) const;

  bool startBoxProcessing(const QString& id,
                          const QString& productionLineId) const;
  bool startPalletProcessing(const QString& id) const;
  bool startOrderProcessing(const QString& id) const;

  bool removeLastProductionLine(void) const;
  bool stopBoxProcessing(const QString& id) const;
  bool stopPalletProcessing(const QString& id) const;
  bool stopOrderProcessing(const QString& id) const;

  bool searchBoxForProductionLine(const QString& orderId,
                                  const QString& productionLineId,
                                  QMap<QString, QString>& boxRecord) const;

  void processingReleaserReturnStatus(
      const TransponderReleaseSystem::ReturnStatus status);

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
  void operationFinished(ExecutionStatus status);
};

#endif  // ORDERCREATIONSYSTEM_H
