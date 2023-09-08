#ifndef ORDERCREATIONSYSTEM_H
#define ORDERCREATIONSYSTEM_H

#include <QApplication>
#include <QDate>
#include <QObject>

#include "Database/database_controller.h"
#include "Database/database_table_model.h"
#include "Database/postgres_controller.h"
#include "firmware_generation_system.h"
#include "transponder_info_model.h"
#include "transponder_release_system.h"

class AdministrationSystem : public QObject {
  Q_OBJECT
 public:
  enum ExecutionStatus {
    NotExecuted,
    DatabaseConnectionError,
    DatabaseQueryError,
    LogicError,
    UnknowError,
    CompletedSuccessfully
  };

 private:
  PostgresController* Database;
  TransponderReleaseSystem* Releaser;
  FirmwareGenerationSystem* Generator;

 public:
  explicit AdministrationSystem(QObject* parent);

 public:
  void applySettings(void);

  void clearDatabaseTable(const QString& tableName);
  void getDatabaseTable(const QString& tableName, DatabaseTableModel* buffer);
  void getCustomResponse(const QString& req, DatabaseTableModel* buffer);
  void initIssuerTable(void);

  void createNewOrder(const QMap<QString, QString>* orderParameters);
  void deleteLastOrder(void);

  void createNewProductionLine(
      const QMap<QString, QString>* productionLineParameters);
  void deleteLastProductionLine(void);
  void linkProductionLineWithBox(const QMap<QString, QString>* linkParameters);

  void releaseTransponder(TransponderInfoModel* seed);
  void confirmTransponder(TransponderInfoModel* seed);
  void refundTransponder(TransponderInfoModel* seed);
  void searchTransponder(TransponderInfoModel* seed);
  void rereleaseTransponder(TransponderInfoModel* seed);

 private:
  void createDatabaseController(void);
  void loadSettings(void);

  bool addOrder(const QMap<QString, QString>* orderParameters) const;
  bool addPallets(const QMap<QString, QString>* orderParameters) const;
  bool addBoxes(const QMap<QString, QString>* orderParameters) const;
  bool addTransponders(const QMap<QString, QString>* orderParameters) const;
  bool addProductionLine(
      const QMap<QString, QString>* productionLineParameters) const;

  bool startBoxAssembling(const QString& id,
                          const QString& productionLineId) const;
  bool startPalletAssembling(const QString& id) const;
  bool startOrderAssembling(const QString& id) const;

  bool removeLastProductionLine(void) const;
  bool stopBoxAssembling(const QString& id) const;
  bool stopPalletAssembling(const QString& id) const;
  bool stopOrderAssembling(const QString& id) const;

  void processingResult(const QString& log, const ExecutionStatus status);

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
  void operationFinished(ExecutionStatus status);
};

#endif  // ORDERCREATIONSYSTEM_H
