#include "postgres_controller.h"

PostgresController::PostgresController(QObject* parent,
                                       const QString& connectionName)
    : IDatabaseController(parent) {
  setObjectName("PostgresController");
  loadSettings();

  ConnectionName = connectionName;

  createDatabaseConnection();

  CurrentRequest = new QSqlQuery(QSqlDatabase::database(ConnectionName));
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
                                     DatabaseTableModel* buffer) {}

void PostgresController::getObuBySerialNumber(const uint32_t serial,
                                              DatabaseTableModel* buffer) {}

void PostgresController::getObuByUCID(const QString& ucid,
                                      DatabaseTableModel* buffer) {}

void PostgresController::getObuListByContextMark(const QString& cm,
                                                 DatabaseTableModel* buffer) {}

void PostgresController::getObuListBySerialNumber(const uint32_t serialBegin,
                                                  const uint32_t serialEnd,
                                                  DatabaseTableModel* buffer) {}

void PostgresController::getObuListByPAN(const uint32_t panBegin,
                                         const uint32_t panEnd,
                                         DatabaseTableModel* buffer) {}

bool PostgresController::getTable(const QString& tableName,
                                  uint32_t rowCount,
                                  DatabaseTableModel* buffer) {
  QString requestText =
      QString("SELECT * FROM %1 ORDER BY \"Id\" ASC;").arg(tableName);
  //  requestText += QString(" ORDER BY PRIMARY KEY DESC LIMIT %1;")
  //                     .arg(QString::number(rowCount));
  emit logging("Отправляемый запрос: " + requestText);

  if (CurrentRequest->exec(requestText)) {
    emit logging("Запрос выполнен. ");
    // Преобразование результатов запроса
    convertResponseToBuffer(buffer);
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
    return false;
  }
}

void PostgresController::execCustomRequest(const QString& req,
                                           DatabaseTableModel* buffer) {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return;
  }

  emit logging(QString("Отправляемый запрос: ") + req);

  // Выполняем запрос
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

void PostgresController::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();

  emit logging("Удаление предыущего подключения к базе данных. ");
  QSqlDatabase::removeDatabase(ConnectionName);

  emit logging("Создание нового подключения к базе данных. ");
  createDatabaseConnection();
}

bool PostgresController::clearTable(const QString& tableName) const {
  QString requestText =
      QString("TRUNCATE TABLE %1 RESTART IDENTITY CASCADE;").arg(tableName);
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  if (CurrentRequest->exec(requestText)) {
    emit logging(QString("Очистка выполнена. "));
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
    return false;
  }
}

int32_t PostgresController::getIdByAttribute(
    const QString& tableName,
    QPair<QString, QString>& attribute) const {
  // Формируем запрос
  QString requestText = QString("SELECT \"Id\" FROM %1 WHERE \"%2\" = '%3';")
                            .arg(tableName)
                            .arg(attribute.first)
                            .arg(attribute.second);
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  if ((CurrentRequest->exec(requestText)) && (CurrentRequest->next())) {
    int32_t id = CurrentRequest->record().value(0).toInt();
    emit logging(
        QString("Получен идентификатор: %1. ").arg(QString::number(id)));
    return id;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
    return -1;
  }
}

int32_t PostgresController::getIdByCondition(const QString& tableName,
                                             const QString& condition,
                                             bool minMaxOption) {
  // Создаем запрос
  QString requestText = "SELECT \"Id\" FROM " + tableName + " WHERE " +
                        condition + " ORDER BY \"Id\" ";
  if (minMaxOption) {
    requestText += "ASC LIMIT 1";
  } else {
    requestText += "DESC LIMIT 1";
  }

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  if ((CurrentRequest->exec(requestText)) && (CurrentRequest->next())) {
    int32_t id = CurrentRequest->record().value(0).toInt();
    emit logging(
        QString("Получен идентификатор: %1. ").arg(QString::number(id)));
    return id;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
    return -1;
  }
}

bool PostgresController::increaseAttributeValue(const QString& tableName,
                                                const QString& attributeName,
                                                const QString& id,
                                                uint32_t value) {
  QString requestText = QString("UPDATE %1 ").arg(tableName);
  requestText += QString("SET \"%1\" = \"%1\" + %2 ")
                     .arg(attributeName)
                     .arg(QString::number(value));
  requestText += QString("WHERE \"Id\" = '%1'").arg(id);
  emit logging("Отправляемый запрос: " + requestText);

  if (CurrentRequest->exec(requestText)) {
    emit logging("Запрос выполнен. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
    return false;
  }
}

bool PostgresController::addTableRecord(const QString& tableName,
                                        QMap<QString, QString>& record) const {
  // Создаем запрос
  QString requestText = "INSERT INTO " + tableName + " (";
  for (int32_t i = 0; i < record.keys().size(); i++) {
    requestText += QString("\"%1\", ").arg(record.keys().at(i));
  }
  requestText.chop(2);
  requestText += ") VALUES (";
  for (int32_t i = 0; i < record.values().size(); i++) {
    requestText += QString("'%1', ").arg(record.values().at(i));
  }
  requestText.chop(2);
  requestText += ");";

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  if (CurrentRequest->exec(requestText)) {
    emit logging("Запись добавлена. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
    return false;
  }
}

bool PostgresController::removeTableLastRecordWithCondition(
    const QString& tableName,
    const QString& condition) const {
  // Создаем запрос
  QString requestText =
      QString(
          "DELETE FROM %1 WHERE \"Id\" = (SELECT \"Id\" FROM %1 ORDER BY "
          "\"Id\" DESC LIMIT 1);")
          .arg(tableName);

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  if ((CurrentRequest->exec(requestText)) && (CurrentRequest->next())) {
    emit logging("Запись удалена. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " +
                 CurrentRequest->lastError().text());
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

void PostgresController::createDatabaseConnection() {
  QSqlDatabase postgres = QSqlDatabase::addDatabase("QPSQL", ConnectionName);

  postgres.setHostName(HostAddress.toString());
  postgres.setPort(Port);
  postgres.setDatabaseName(DatabaseName);
  postgres.setUserName(UserName);
  postgres.setPassword(Password);
}

void PostgresController::convertResponseToBuffer(DatabaseTableModel* buffer) {
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
