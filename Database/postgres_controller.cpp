#include "postgres_controller.h"

PostgresController::PostgresController(QObject* parent)
    : DatabaseControllerInterface(parent) {
  setObjectName("PostgresController");

  HostAddress = POSTGRES_SERVER_DEFAULT_IP;
  Port = POSTGRES_SERVER_DEFAULT_PORT;
  DatabaseName = POSTGRES_DATABASE_DEFAULT_NAME;
  UserName = POSTGRES_SERVER_DEFAULT_USER_NAME;
  Password = POSTGRES_SERVER_DEFAULT_PASSWORD;

  createDatabase();

  CurrentRequest = nullptr;
}

void PostgresController::connect() {
  if (Postgres.isOpen()) {
    emit logging("Соединение с Postgres уже установлено. ");
    return;
  }

  if (Postgres.open()) {
    emit logging("Соединение с Postgres установлено. ");
  } else {
    emit logging(Postgres.lastError().text());
    // Дополнительные действия по обработке ошибки
    emit logging("Соединение с Postgres не может быть установлено. ");
  }
}

void PostgresController::disconnect() {
  Postgres.close();
  emit logging("Соединение с Postgres отключено. ");
}

void PostgresController::getObuByPAN(const QString& pan,
                                     QVector<QString>* result) {}

void PostgresController::getObuBySerialNumber(const uint32_t serial,
                                              QVector<QString>* result) {}

void PostgresController::getObuByUCID(const QString& ucid,
                                      QVector<QString>* result) {}

void PostgresController::getObuListByContextMark(
    const QString& cm,
    QVector<QVector<QString>>* result) {}

void PostgresController::getObuListBySerialNumber(
    const uint32_t serialBegin,
    const uint32_t serialEnd,
    QVector<QVector<QString>>* result) {}

void PostgresController::getObuListByPAN(const uint32_t panBegin,
                                         const uint32_t panEnd,
                                         QVector<QVector<QString>>* result) {}

void PostgresController::execCustomRequest(const QString& req,
                                           QVector<QVector<QString>>* result) {
  if (!Postgres.isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return;
  }

  delete CurrentRequest;
  CurrentRequest = new QSqlQuery(Postgres);

  emit logging("Отправляемый запрос: " + req);

  if (CurrentRequest->exec(req)) {
    emit logging("Ответ получен. ");
    // Обработка результатов запроса

    QSqlRecord record = CurrentRequest->record();

    for (int32_t i = 0; i < CurrentRequest->record().count(); i++)
      emit logging(CurrentRequest->record().fieldName(i));

    while (CurrentRequest->next()) {
      emit logging("Значение: " + CurrentRequest->value(0).toString());
    }
    //    emit operationExecutionEnd(Complete,
    //                               CurrentRequest->value());
    // SELECT column_name FROM information_schema.columns WHERE
    // table_name='obulist';
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
  }
}

void PostgresController::applySettings(UserSettings* settings) {}

void PostgresController::createDatabase() {
  Postgres = QSqlDatabase::addDatabase("QPSQL", "postgres");

  Postgres.setHostName(HostAddress.toString());
  Postgres.setPort(Port);
  Postgres.setDatabaseName(DatabaseName);
  Postgres.setUserName(UserName);
  Postgres.setPassword(Password);
}

void PostgresController::checkConnection() {}
