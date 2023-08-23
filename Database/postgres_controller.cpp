#include "postgres_controller.h"

PostgresController::PostgresController(QObject* parent,
                                       const QString& connectionName)
    : IDatabaseController(parent) {
  setObjectName("PostgresController");
  loadSettings();

  ConnectionName = connectionName;

  createDatabaseConnection();

  CurrentRequest = nullptr;
}

PostgresController::~PostgresController() {
  delete CurrentRequest;

  // Удаляем соединение
  QSqlDatabase::removeDatabase(ConnectionName);
}

void PostgresController::connect() {
  if (QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres уже установлено. ");
    return;
  }

  if (!QSqlDatabase::database(ConnectionName).open()) {
    emit logging(
        QString("Соединение с Postgres не может быть установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return;
  }

  emit logging("Соединение с Postgres установлено. ");
}

void PostgresController::disconnect() {
  QSqlDatabase::database(ConnectionName).close();
  emit logging("Соединение с Postgres отключено. ");
}

bool PostgresController::isConnected() {
  return QSqlDatabase::database(ConnectionName).isOpen();
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
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return;
  }

  // Создаем запрос
  CurrentRequest = new QSqlQuery(QSqlDatabase::database(ConnectionName));

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

  // Удаляем запрос
  delete CurrentRequest;
}

void PostgresController::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();

  emit logging("Удаление предыущего подключения к базе данных. ");
  QSqlDatabase::removeDatabase(ConnectionName);

  emit logging("Создание нового подключения к базе данных. ");
  createDatabaseConnection();
}

bool PostgresController::addOrder(const QString& issuerName) {
  emit logging(QString("Добавление нового заказа для \"%1\"").arg(issuerName));

  //  INSERT INTO Customers(Age, FirstName) VALUES(30, 'John');
  //  INSERT INTO Orders(CustomerId, Quantity)
  //      VALUES((SELECT Id FROM Customers WHERE FirstName = 'John'), 5);

  // Создаем запрос
  QString query =
      QString(
          "UPDATE public.issuers SET \"OrderQuantity\" = \"OrderQuantity\" + 1 "
          "WHERE \"Name\" = '%1';")
          .arg(issuerName);
  emit logging("Текст запроса: ");
  emit logging(query);

  // Выполняем запрос
  QSqlQuery CurrentRequest(QSqlDatabase::database(ConnectionName));
  if (CurrentRequest.exec(query)) {
    emit logging("Заказ добавлен. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest.lastError().text());
    return false;
  }
}

void PostgresController::loadSettings() {
  // Загружаем настройки
  QSettings settings(ORGANIZATION_NAME, PROGRAM_NAME);

  HostAddress = settings.value("Database/Server/Ip").toString();
  Port = settings.value("Database/Server/Port").toInt();
  DatabaseName = settings.value("Database/Name").toString();
  UserName = settings.value("Database/User/Name").toString();
  Password = settings.value("Database/User/Password").toString();
}

void PostgresController::getTable(const QString& tableName,
                                  uint32_t rowCount,
                                  DatabaseBuffer* buffer) {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return;
  }

  delete CurrentRequest;
  CurrentRequest = new QSqlQuery(QSqlDatabase::database(ConnectionName));

  QString requestText("SELECT * FROM ");
  requestText += tableName;
  requestText += QString(";");
  //  requestText += QString(" ORDER BY PRIMARY KEY DESC LIMIT %1;")
  //                     .arg(QString::number(rowCount));
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

void PostgresController::createDatabaseConnection() {
  QSqlDatabase postgres = QSqlDatabase::addDatabase("QPSQL", ConnectionName);

  postgres.setHostName(HostAddress.toString());
  postgres.setPort(Port);
  postgres.setDatabaseName(DatabaseName);
  postgres.setUserName(UserName);
  postgres.setPassword(Password);
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
