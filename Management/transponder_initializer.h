#ifndef TRANSPONDERINITIALIZER_H
#define TRANSPONDERINITIALIZER_H

#include <QObject>

#include "Database/database_buffer.h"
#include "Database/database_controller.h"
#include "Database/postgres_controller.h"
#include "issuer_order.h"

class TransponderInitializer : public QObject {
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
  IDatabaseController* Database;

 public:
  explicit TransponderInitializer(QObject* parent);

 public slots:
  void proxyLogging(const QString& log);
  void applySettings(void);

  void createDatabaseController(void);
  void getDatabaseTable(const QString& tableName, DatabaseBuffer* buffer);
  void getCustomResponse(const QString& req, DatabaseBuffer* buffer);
  void createNewOrder(IssuerOrder* order);

 signals:
  void logging(const QString& log);
  void operationFinished(ExecutionStatus status);
};

#endif  // TRANSPONDERINITIALIZER_H
