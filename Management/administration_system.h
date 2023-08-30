#ifndef ORDERCREATIONSYSTEM_H
#define ORDERCREATIONSYSTEM_H

#include <QApplication>
#include <QDate>
#include <QObject>

#include "Database/database_buffer.h"
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
  bool DatabaseLogOption;

 public:
  explicit AdministrationSystem(QObject* parent);

 public slots:
  void proxyLogging(const QString& log);
  void applySettings(void);

  void createDatabaseController(void);
  void clearDatabaseTable(const QString& tableName);
  void getDatabaseTable(const QString& tableName, DatabaseTableModel* buffer);
  void getCustomResponse(const QString& req, DatabaseTableModel* buffer);

  void createNewOrder(const QMap<QString, QString>* orderParameters);
  void deleteLastOrder(void);

  void createNewProductionLine(
      const QMap<QString, QString>* productionLineParameters);
  void deleteLastProductionLines(void);

  void initIssuerTable(void);

 private:
  void loadSettings(void);

  bool addOrder(const QMap<QString, QString>* orderParameters);
  bool addPallets(const QMap<QString, QString>* orderParameters);
  bool addBoxes(const QMap<QString, QString>* orderParameters);
  bool addTransponders(const QMap<QString, QString>* orderParameters);
  void processingResult(const QString& log, const ExecutionStatus status);
  void init(void);

 signals:
  void logging(const QString& log);
  void operationFinished(ExecutionStatus status);
};

#endif  // ORDERCREATIONSYSTEM_H
