#include "postgres_controller.h"

PostgresController::PostgresController(QObject* parent,
                                       const QString& connectionName)
    : DatabaseControllerInterface(parent) {
  setObjectName("PostgresController");

  ConnectionName = connectionName;
  HostAddress = POSTGRES_SERVER_DEFAULT_IP;
  Port = POSTGRES_SERVER_DEFAULT_PORT;
  DatabaseName = POSTGRES_DATABASE_DEFAULT_NAME;
  UserName = POSTGRES_SERVER_DEFAULT_USER_NAME;
  Password = POSTGRES_SERVER_DEFAULT_PASSWORD;

  createDatabase();

  CurrentRequest = nullptr;
}

PostgresController::~PostgresController() {
  if (Postgres.isValid()) {
    // Закрываем соединение
    Postgres.close();
    // Удаляем соединение
    Postgres.removeDatabase(ConnectionName);
  }
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
                                     DatabaseBuffer* buffer) {}

void PostgresController::getObuBySerialNumber(const uint32_t serial,
                                              DatabaseBuffer* buffer) {}

void PostgresController::getObuByUCID(const QString& ucid,
                                      DatabaseBuffer* buffer) {}

void PostgresController::getObuListByContextMark(const QString& cm,
                                                 DatabaseBuffer* buffer) {}

void PostgresController::getObuListBySerialNumber(const uint32_t serialBegin,
                                                  const uint32_t serialEnd,
                                                  DatabaseBuffer* buffer) {}

void PostgresController::getObuListByPAN(const uint32_t panBegin,
                                         const uint32_t panEnd,
                                         DatabaseBuffer* buffer) {}

void PostgresController::execCustomRequest(const QString& req,
                                           DatabaseBuffer* buffer) {
  if (!Postgres.isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return;
  }

  delete CurrentRequest;
  CurrentRequest = new QSqlQuery(Postgres);

  emit logging("Отправляемый запрос: " + req);

  if (CurrentRequest->exec(req)) {
    emit logging("Ответ получен. ");
    // Преобразование результатов запроса
    convertResponseToBuffer(buffer);
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
  }
}

void PostgresController::applySettings(QSettings* settings) {
  HostAddress = settings->value("Database/Server/HostAddress").toString();
  Port = settings->value("Database/Server/Port").toInt();
  DatabaseName = settings->value("Database/Name").toString();
  UserName = settings->value("Database/User/Name").toString();
  Password = settings->value("Database/User/Password").toString();
}

void PostgresController::getTable(const QString& tableName,
                                  uint32_t rowCount,
                                  DatabaseBuffer* buffer) {
  if (!Postgres.isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return;
  }

  delete CurrentRequest;
  CurrentRequest = new QSqlQuery(Postgres);

  QString requestText("SELECT * FROM ");
  requestText += tableName;
  requestText += QString(" ORDER BY PRIMARY KEY DESC LIMIT %1;")
                     .arg(QString::number(rowCount));
  emit logging("Отправляемый запрос: " + requestText);

  if (CurrentRequest->exec(requestText)) {
    emit logging("Ответ получен. ");
    // Преобразование результатов запроса
    convertResponseToBuffer(buffer);
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
  }
}

void PostgresController::createDatabase() {
  Postgres = QSqlDatabase::addDatabase("QPSQL", ConnectionName);

  Postgres.setHostName(HostAddress.toString());
  Postgres.setPort(Port);
  Postgres.setDatabaseName(DatabaseName);
  Postgres.setUserName(UserName);
  Postgres.setPassword(Password);
}

void PostgresController::convertResponseToBuffer(DatabaseBuffer* buffer) {
  int32_t i = 0, j = 0;

  QVector<QVector<QString>*>* data = new QVector<QVector<QString>*>();
  QVector<QString>* headers = new QVector<QString>();

  // Заголовки таблицы
  for (i = 0; i < CurrentRequest->record().count(); i++)
    headers->append(CurrentRequest->record().fieldName(i));

  // Данные таблицы
  i = 0;
  while (CurrentRequest->next()) {
    // Получение информации о столбцах записи
    QSqlRecord record = CurrentRequest->record();

    QVector<QString>* row = new QVector<QString>;

    // Перебор столбцов записи и вывод в лог
    for (j = 0; j < record.count(); ++j) {
      row->append(CurrentRequest->value(j).toString());
    }
    data->append(row);

    i++;
  }

  // Строим буффер для отображения
  buffer->build(headers, data);
}
