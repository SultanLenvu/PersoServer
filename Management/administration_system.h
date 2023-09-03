#ifndef ORDERCREATIONSYSTEM_H
#define ORDERCREATIONSYSTEM_H

#include <QApplication>
#include <QDate>
#include <QObject>

#include "Database/database_table_model.h"
#include "Database/database_controller.h"
#include "Database/postgres_controller.h"

class AdministrationSystem : public QObject {
  Q_OBJECT
 public:
  enum ExecutionStatus {
    NotExecuted,
    DatabaseConnectionError,
    DatabaseQueryError,
    UnknowError,
    CompletedSuccessfully
  };

 private:
  PostgresController* Database;

 public:
  explicit AdministrationSystem(QObject* parent);

 public slots:
  void proxyLogging(const QString& log);
  void applySettings(void);

  void clearDatabaseTable(const QString& tableName);
  void getDatabaseTable(const QString& tableName, DatabaseTableModel* buffer);
  void getCustomResponse(const QString& req, DatabaseTableModel* buffer);
  void initIssuerTable(void);

  void createNewOrder(const QMap<QString, QString>* orderParameters);
  void deleteLastOrder(void);

  void createNewProductionLine(
      const QMap<QString, QString>* productionLineParameters);
  void deleteLastProductionLines(void);

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

 signals:
  void logging(const QString& log) const;
  void operationFinished(ExecutionStatus status);
};

#endif  // ORDERCREATIONSYSTEM_H
