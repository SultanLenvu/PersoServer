#ifndef ORDERCREATIONSYSTEM_H
#define ORDERCREATIONSYSTEM_H

#include <QApplication>
#include <QObject>

#include "Database/database_buffer.h"
#include "Database/database_controller.h"
#include "Database/postgres_controller.h"
#include "issuer_order.h"

class OrderCreationSystem : public QObject {
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
  explicit OrderCreationSystem(QObject* parent);

 public slots:
  void proxyLogging(const QString& log);
  void applySettings(void);

  void createDatabaseController(void);
  void getDatabaseTable(const QString& tableName, DatabaseBuffer* buffer);
  void getCustomResponse(const QString& req, DatabaseBuffer* buffer);
  void createNewOrder(IssuerOrder* order);

 private:
  void init(void);

 signals:
  void logging(const QString& log);
  void operationFinished(ExecutionStatus status);
};

class OCSBuilder : public QObject {
  Q_OBJECT
 private:
  OrderCreationSystem* BuildedObject;

 public:
  explicit OCSBuilder(void);
  OrderCreationSystem* buildedObject() const;

 public slots:
  void build(void);

 signals:
  void completed(void);
};

#endif  // ORDERCREATIONSYSTEM_H
