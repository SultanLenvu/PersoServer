#include "transponder_initializer.h"

TransponderInitializer::TransponderInitializer(QObject* parent)
    : QObject(parent) {
  setObjectName("TransponderInitializer");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");
}

void TransponderInitializer::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres controller - " + log);
  else
    emit logging("Unknown - " + log);
}

void TransponderInitializer::applySettings() {
  emit logging("Применение новых настроек. ");
  Database->applySettings();
}

void TransponderInitializer::createDatabaseController() {
  // Создаем контроллер базы данных
  Database = new PostgresController(this, "InitializerConnection");
  connect(Database, &IDatabaseController::logging, this,
          &TransponderInitializer::proxyLogging);
}

void TransponderInitializer::getDatabaseTable(const QString& tableName,
                                              DatabaseBuffer* buffer) {
  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

  Database->getTable(tableName, 10, buffer);

  if (buffer->isEmpty()) {
    emit logging("Ошибка при выполнении запроса к базе данных. ");
    emit operationFinished(DatabaseQueryError);
  } else {
    emit logging("Операция успешно выполнена. ");
    emit operationFinished(CompletedSuccessfully);
  }
}

void TransponderInitializer::getCustomResponse(const QString& req,
                                               DatabaseBuffer* buffer) {
  Database->execCustomRequest(req, buffer);
}
