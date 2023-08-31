#include "postgres_controller.h"

PostgresController::PostgresController(QObject* parent,
                                       const QString& connectionName)
    : IDatabaseController(parent) {
  setObjectName("PostgresController");
  loadSettings();

  ConnectionName = connectionName;
}

PostgresController::~PostgresController() {
  // Удаляем соединение
  QSqlDatabase::removeDatabase(ConnectionName);
}

bool PostgresController::connect() {
  if (QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres уже установлено. ");
    return true;
  }

  // Создаем соединение
  createDatabaseConnection();

  if (!QSqlDatabase::database(ConnectionName).open()) {
    emit logging(
        QString("Соединение с Postgres не может быть установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  emit logging("Соединение с Postgres установлено. ");
  return true;
}

void PostgresController::disconnect(bool resultOption) {
  // Удаляем соединение
  QSqlDatabase::removeDatabase(ConnectionName);

  if (QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres не отключено. ");
  } else {
    emit logging("Соединение с Postgres отключено. ");
  }
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

  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запрос выполнен. ");
    // Преобразование результатов запроса
    convertResponseToBuffer(request, buffer);
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::execCustomRequest(const QString& req,
                                           DatabaseTableModel* buffer) {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging("Соединение с Postgres не установлено. ");
    return false;
  }

  emit logging(QString("Отправляемый запрос: ") + req);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(req)) {
    emit logging("Ответ получен. ");
    // Преобразование результатов запроса
    convertResponseToBuffer(request, buffer);
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
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
  // Формируем запрос
  QString requestText =
      QString("TRUNCATE TABLE %1 RESTART IDENTITY CASCADE;").arg(tableName);
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging(QString("Очистка выполнена. "));
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

int32_t PostgresController::getLastId(const QString& tableName) const {
  int32_t id = 0;
  int32_t rowCount = -1;

  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Формируем запрос
  QString requestText =
      QString("SELECT \"Id\" FROM %1 ORDER BY \"Id\" DESC LIMIT 1;")
          .arg(tableName);
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if ((request.exec(requestText)) && (request.next())) {
    id = request.record().value(0).toInt();
    emit logging(
        QString("Получен идентификатор: %1. ").arg(QString::number(id)));
    return id;
  } else {
    emit logging(
        QString("Проверка на наличие записей в таблице %1. ").arg(tableName));
    requestText = QString("SELECT COUNT(*) FROM %1;").arg(tableName);
    emit logging(QString("Отправляемый запрос: ") + requestText);

    // Выполняем запрос
    if ((request.exec(requestText)) && (request.next())) {
      rowCount = request.value(0).toInt();
      emit logging(
          QString("Количество записей: %1. ").arg(QString::number(rowCount)));

      if (rowCount == 0) {
        emit logging(
            QString("Таблица пустая. Возвращаем нулевой идентификатор. "));
        if (tableName == "transponders") {
          return TRANSPONDER_ID_START_SHIFT;
        } else {
          return 0;
        }
      } else {
        emit logging(
            "Таблица не пустая, искомый идентификатор не был найден. ");
        return -1;
      }

    } else {
      // Обработка ошибки выполнения запроса
      emit logging("Ошибка выполнения запроса: " + request.lastError().text());
      return -1;
    }
  }
}

int32_t PostgresController::getFirstIdByAttribute(
    const QString& tableName,
    QPair<QString, QString>& attribute) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Формируем запрос
  QString requestText = QString(
                            "SELECT \"Id\" FROM %1 WHERE \"%2\" = '%3' ORDER "
                            "BY \"Id\" ASC LIMIT 1;")
                            .arg(tableName)
                            .arg(attribute.first)
                            .arg(attribute.second);
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if ((request.exec(requestText)) && (request.next())) {
    int32_t id = request.record().value(0).toInt();
    emit logging(
        QString("Получен идентификатор: %1. ").arg(QString::number(id)));
    return id;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return -1;
  }
}

int32_t PostgresController::getFirstIdWithCondition(const QString& tableName,
                                                    const QString& condition,
                                                    bool minMaxOption) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Формируем запрос
  QString requestText = "SELECT \"Id\" FROM " + tableName + " WHERE " +
                        condition + " ORDER BY \"Id\" ";
  if (minMaxOption) {
    requestText += "ASC LIMIT 1";
  } else {
    requestText += "DESC LIMIT 1";
  }

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if ((request.exec(requestText)) && (request.next())) {
    int32_t id = request.record().value(0).toInt();
    emit logging(
        QString("Получен идентификатор: %1. ").arg(QString::number(id)));
    return id;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return -1;
  }
}

bool PostgresController::increaseAttributeValue(const QString& tableName,
                                                const QString& attributeName,
                                                const QString& id,
                                                uint32_t value) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Формируем запрос
  QString requestText = QString("UPDATE %1 ").arg(tableName);
  requestText += QString("SET \"%1\" = \"%1\" + %2 ")
                     .arg(attributeName)
                     .arg(QString::number(value));
  requestText += QString("WHERE \"Id\" = '%1'").arg(id);
  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запрос выполнен. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::addRecord(const QString& tableName,
                                   QMap<QString, QString>& record) const {
  // Проверка подключения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

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
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запись добавлена. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::getRecordById(const QString& tableName,
                                       const uint32_t id,
                                       QMap<QString, QString>& record) const {
  // Формируем запрос
  QString requestText = QString("SELECT * FROM public.%1 WHERE \"Id\" = '%2';")
                            .arg(tableName)
                            .arg(QString::number(id));
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText) && (request.next())) {
    emit logging(QString("Запрос выполнен. "));
    convertResponseToMap(request, record);
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::getRecordByPart(const QString& tableName,
                                         QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("SELECT * FROM %1 WHERE ").arg(tableName);
  for (int32_t i = 0; i < record.size(); i++) {
    requestText += QString("\"%1\" = '%2' AND ")
                       .arg(record.keys().at(i))
                       .arg(record.values().at(i));
  }
  requestText.chop(4);
  requestText += " ORDER BY \"Id\" ASC LIMIT 1;";

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if ((request.exec(requestText)) && (request.next())) {
    emit logging("Запрос выполнен. ");
    convertResponseToMap(request, record);
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::updateRecord(const QString& tableName,
                                      QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("UPDATE public.%1 SET ").arg(tableName);
  for (int32_t i = 0; i < record.size(); i++) {
    requestText += QString("\"%1\" = '%2', ")
                       .arg(record.keys()[i])
                       .arg(record.values()[i]);
  }
  requestText.chop(2);
  requestText += QString(" WHERE \"Id\" = '%1';").arg(record.value("Id"));

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запрос выполнен. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::removeRecordById(const QString& tableName,
                                          const uint32_t id) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("DELETE FROM public.%1 WHERE \"Id\" = '%2';")
                            .arg(tableName)
                            .arg(QString::number(id));

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запрос выполнен. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::getLastRecord(const QString& tableName,
                                       QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Формируем запрос
  QString requestText =
      QString("SELECT \"Id\" FROM %1 ORDER BY \"Id\" DESC LIMIT 1;")
          .arg(tableName);
  emit logging(QString("Отправляемый запрос: ") + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if ((request.exec(requestText)) && (request.next())) {
    emit logging("Запрос выполнен. ");
    convertResponseToMap(request, record);
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::removeLastRecord(const QString& tableName) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText =
      QString(
          "DELETE FROM %1 WHERE \"Id\" = (SELECT \"Id\" FROM %1 ORDER BY "
          "\"Id\" DESC LIMIT 1);")
          .arg(tableName);

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запись удалена. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::removeLastRecordWithCondition(
    const QString& tableName,
    const QString& condition) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    emit logging(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText =
      QString(
          "DELETE FROM %1 WHERE \"Id\" = (SELECT \"Id\" FROM %1 ORDER BY "
          "\"Id\" DESC LIMIT 1) AND %2;")
          .arg(tableName)
          .arg(condition);

  emit logging("Отправляемый запрос: " + requestText);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    emit logging("Запись удалена. ");
    return true;
  } else {
    // Обработка ошибки выполнения запроса
    emit logging("Ошибка выполнения запроса: " + request.lastError().text());
    return false;
  }
}

bool PostgresController::openTransaction() const {}

bool PostgresController::closeTransaction() const {}

bool PostgresController::abortTransaction() const {}

void PostgresController::loadSettings() {
  // Загружаем настройки
  QSettings settings;

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

void PostgresController::convertResponseToBuffer(
    QSqlQuery& request,
    DatabaseTableModel* buffer) const {
  int32_t i = 0, j = 0;

  QVector<QVector<QString>*>* data = new QVector<QVector<QString>*>();
  QVector<QString>* headers = new QVector<QString>();

  // Заголовки таблицы
  for (i = 0; i < request.record().count(); i++)
    headers->append(request.record().fieldName(i));

  // Данные таблицы
  i = 0;
  while (request.next()) {
    // Получение информации о столбцах записи
    QSqlRecord record = request.record();

    QVector<QString>* row = new QVector<QString>;

    // Перебор столбцов записи и вывод в лог
    for (j = 0; j < record.count(); ++j) {
      row->append(request.value(j).toString());
    }
    data->append(row);

    i++;
  }

  // Строим буффер для отображения
  buffer->build(headers, data);
}

void PostgresController::convertResponseToMap(
    QSqlQuery& request,
    QMap<QString, QString>& record) const {
  record.clear();
  emit logging("Полученная запись: ");
  for (int32_t i = 0; i < request.record().count(); i++) {
    record.insert(request.record().fieldName(i), request.value(i).toString());
  }

  //  QList<QString> keys = record.keys();
  //  QList<QString> values = record.values();

  //  for (int i = 0; i < keys.size(); ++i) {
  //    emit logging(QString("%1 = %2").arg(keys.at(i)).arg(values.at(i)));
  //  }
}
