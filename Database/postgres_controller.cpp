#include "postgres_controller.h"

PostgresController::PostgresController(QObject* parent,
                                       const QString& connectionName)
    : IDatabaseController(parent) {
  loadSettings();

  ConnectionName = connectionName;
}

PostgresController::~PostgresController() {
  // Удаляем соединение
  QSqlDatabase::removeDatabase(ConnectionName);
}

bool PostgresController::connect() {
  if (QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres уже установлено. ");
    return true;
  }

  // Создаем соединение
  createDatabaseConnection();

  if (!QSqlDatabase::database(ConnectionName).open()) {
    sendLog(
        QString("Соединение с Postgres не может быть установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }
  sendLog("Соединение с Postgres установлено. ");

  return true;
}

void PostgresController::disconnect(void) {
  // Удаляем соединение
  QSqlDatabase::removeDatabase(ConnectionName);

  if (QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres не отключено. ");
  } else {
    sendLog("Соединение с Postgres отключено. ");
  }
}

bool PostgresController::openTransaction() const {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres не установлено. ");
    return false;
  }

  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec("BEGIN;")) {
    return true;
  } else {
    sendLog("Получена ошибка при открытии транзакции: " +
            request.lastError().text());
    return false;
  }
}

bool PostgresController::closeTransaction() const {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres не установлено. ");
    return false;
  }

  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec("COMMIT;")) {
    return true;
  } else {
    sendLog(request.lastError().text());
    return false;
  }
}

bool PostgresController::abortTransaction() const {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres не установлено. ");
    return false;
  }

  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec("ROLLBACK;")) {
    return true;
  } else {
    sendLog(request.lastError().text());
    return false;
  }
}

bool PostgresController::getTable(const QString& tableName,
                                  uint32_t rowCount,
                                  DatabaseTableModel* buffer) const {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres не установлено. ");
    return false;
  }

  QString requestText =
      QString("SELECT * FROM public.%1 ORDER BY id ASC;").arg(tableName);

  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запрос выполнен. ");
    //    if (!request.is()) {
    //      sendLog("Ответные данные получены. ");
    convertResponseToBuffer(request, buffer);
    //    } else {
    //      sendLog("Ответные данные не получены. ");
    //      buffer->clear();
    //    }
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::execCustomRequest(const QString& req,
                                           DatabaseTableModel* buffer) const {
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Соединение с Postgres не установлено. ");
    return false;
  }

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(req)) {
    sendLog("Запрос выполнен. ");
    if (request.next()) {
      sendLog("Ответные данные получены. ");
      convertResponseToBuffer(request, buffer);
    } else {
      sendLog("Ответные данные не получены. ");
      buffer->clear();
    }
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + req);
    return false;
  }
}

void PostgresController::applySettings() {
  sendLog("Применение новых настроек. ");
  loadSettings();

  if (QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog("Удаление предыущего подключения к базе данных. ");
    QSqlDatabase::removeDatabase(ConnectionName);

    sendLog("Создание нового подключения к базе данных. ");
    createDatabaseConnection();
  }
}

bool PostgresController::clearTable(const QString& tableName) const {
  // Формируем запрос
  QString requestText = QString("DELETE FROM public.%1;").arg(tableName);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog(QString("Очистка таблицы выполнена. "));
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::addRecord(const QString& tableName,
                                   QMap<QString, QString>& record) const {
  // Проверка подключения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("INSERT INTO public.%1 (").arg(tableName);
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    requestText += QString("%1, ").arg(it.key());
  }
  requestText.chop(2);
  requestText += ") VALUES (";
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    if (it.value() != "NULL") {
      requestText += QString("'%1', ").arg(it.value());
    } else {
      requestText += QString("%1, ").arg(it.value());
    }
  }
  requestText.chop(2);
  requestText += ");";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запись добавлена. ");
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::getRecordById(const QString& tableName,
                                       QMap<QString, QString>& record) const {
  // Формируем запрос
  QString requestText = QString("SELECT ");
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    requestText += QString("%1, ").arg(it.key());
  }
  requestText.chop(2);
  requestText += QString(" FROM public.%1 WHERE id = '%2';")
                     .arg(tableName, record.value("id"));

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog(QString("Запрос выполнен. "));
    if (request.next()) {
      sendLog("Ответные данные получены. ");
      convertResponseToMap(request, record);
    } else {
      record.clear();
      sendLog("Ответные данные не получены. ");
    }
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::getRecordByPart(const QString& tableName,
                                         QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("SELECT ");
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    requestText += QString("%1, ").arg(it.key());
  }

  requestText.chop(2);
  requestText += QString(" FROM public.%1 WHERE ").arg(tableName);
  bool flag = false;
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    if (it.value().size() > 0) {
      flag = true;
      requestText += QString("%1 = '%2' AND ").arg(it.key(), it.value());
    }
  }

  if (flag) {
    requestText.chop(4);
  }
  requestText += " ORDER BY id ASC LIMIT 1;";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запрос выполнен. ");
    if (request.next()) {
      sendLog("Ответные данные получены. ");
      convertResponseToMap(request, record);
    } else {
      record.clear();
      sendLog("Ответные данные не получены. ");
    }
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::getLastRecord(const QString& tableName,
                                       QMap<QString, QString>& record) const {
  int32_t rowCount = 0;

  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Формируем запрос
  QString requestText = QString("SELECT ");
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    requestText += QString("%1, ").arg(it.key());
  }
  requestText.chop(2);
  requestText +=
      QString(" FROM public.%1 ORDER BY id DESC LIMIT 1;").arg(tableName);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запрос выполнен. ");
    if (request.next()) {
      sendLog("Ответные данные получены. ");
      convertResponseToMap(request, record);
    } else {
      record.clear();
      sendLog("Ответные данные не получены. ");

      sendLog(
          QString("Проверка на наличие записей в таблице %1. ").arg(tableName));
      requestText = QString("SELECT COUNT(*) FROM %1;").arg(tableName);

      // Выполняем запрос
      if ((request.exec(requestText)) && (request.next())) {
        rowCount = request.value(0).toInt();
        sendLog(
            QString("Количество записей: %1. ").arg(QString::number(rowCount)));

        if (rowCount == 0) {
          sendLog(
              QString("Таблица пустая. Возвращаем нулевой идентификатор. "));
          if (tableName == "transponders") {
            record.insert("id", QString::number(TRANSPONDER_ID_START_SHIFT));
          } else {
            record.insert("id", "0");
          }
        } else {
          sendLog("Таблица не пустая, искомый идентификатор не был найден. ");
          return false;
        }
      } else {  // Обработка ошибки выполнения запроса
        sendLog(request.lastError().text());
        sendLog("Отправленный запрос: " + requestText);
        return false;
      }
    }
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  return true;
}

bool PostgresController::getMergedRecordById(
    const QStringList& tables,
    const QStringList& foreignKeys,
    QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("SELECT ");
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    requestText += QString("%1, ").arg(it.key());
  }

  requestText.chop(2);
  requestText += QString(" FROM public.%1 ").arg(tables.first());
  for (int32_t i = 0; i < tables.size() - 1; i++) {
    requestText += QString("JOIN %1 ON %2.%3 = %1.id ")
                       .arg(tables.at(i + 1), tables.at(i), foreignKeys.at(i));
  }
  requestText += QString("WHERE %1.id = '%2';")
                     .arg(tables.first(),
                          record.value(QString("%1.id").arg(tables.first())));

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запрос выполнен. ");
    if (request.next()) {
      sendLog("Ответные данные получены. ");
      convertResponseToMap(request, record);
    } else {
      record.clear();
      sendLog("Ответные данные не получены. ");
    }
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::getMergedRecordByPart(
    const QStringList& tables,
    const QStringList& foreignKeys,
    QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("SELECT ");
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    requestText += QString("%1, ").arg(it.key());
  }

  requestText.chop(2);
  requestText += QString(" FROM public.%1 ").arg(tables.first());
  for (int32_t i = 0; i < tables.size() - 1; i++) {
    requestText += QString("JOIN %1 ON %2.%3 = %1.id ")
                       .arg(tables.at(i + 1), tables.at(i), foreignKeys.at(i));
  }

  requestText += "WHERE ";
  bool flag = false;
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    if (it.value().size() > 0) {
      flag = true;
      requestText += QString("%1 = '%2' AND ").arg(it.key(), it.value());
    }
  }
  if (flag) {
    requestText.chop(4);
  }
  requestText += QString(" ORDER BY %1.id ASC LIMIT 1;").arg(tables.first());

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запрос выполнен. ");
    if (request.next()) {
      sendLog("Ответные данные получены. ");
      convertResponseToMap(request, record);
    } else {
      record.clear();
      sendLog("Ответные данные не получены. ");
    }
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::updateRecordById(
    const QString& tableName,
    QMap<QString, QString>& record) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("UPDATE public.%1 SET ").arg(tableName);
  for (QMap<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    if (it.value() != "NULL") {
      requestText += QString("%1 = '%2', ").arg(it.key(), it.value());
    } else {
      requestText += QString("%1 = %2, ").arg(it.key(), it.value());
    }
  }

  requestText.chop(2);
  requestText += QString(" WHERE id = '%1';").arg(record.value("id"));

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запись обновлена. ");
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::updateAllRecordsByPart(
    const QString& tableName,
    QMap<QString, QString>& conditions,
    QMap<QString, QString>& newValues) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("UPDATE public.%1 SET ").arg(tableName);
  for (QMap<QString, QString>::const_iterator it = newValues.constBegin();
       it != newValues.constEnd(); it++) {
    if (it.value() != "NULL") {
      requestText += QString("%1 = '%2', ").arg(it.key(), it.value());
    } else {
      requestText += QString("%1 = %2, ").arg(it.key(), it.value());
    }
  }
  requestText.chop(2);

  requestText += " WHERE ";
  bool flag = false;
  for (QMap<QString, QString>::const_iterator it = conditions.constBegin();
       it != conditions.constEnd(); it++) {
    if (it.value().size() > 0) {
      flag = true;
      requestText += QString("%1 = '%2' AND ").arg(it.key(), it.value());
    }
  }
  if (flag) {
    requestText.chop(4);
  }
  requestText += ";";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Записи обновлены. ");
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::removeRecordById(const QString& tableName,
                                          const uint32_t id) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText = QString("DELETE FROM public.%1 WHERE id = '%2';")
                            .arg(tableName, QString::number(id));

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запись удалена. ");
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::removeLastRecordById(const QString& tableName) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText =
      QString(
          "DELETE FROM %1 WHERE id = (SELECT id FROM %1 ORDER BY "
          "id DESC LIMIT 1);")
          .arg(tableName);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Запись удалена. ");
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

bool PostgresController::removeLastRecordByCondition(
    const QString& tableName,
    QMap<QString, QString>& condition) const {
  // Проверка соединения
  if (!QSqlDatabase::database(ConnectionName).isOpen()) {
    sendLog(
        QString("Соединение с Postgres не установлено. Ошибка: %1.")
            .arg(QSqlDatabase::database(ConnectionName).lastError().text()));
    return false;
  }

  // Создаем запрос
  QString requestText =
      QString(
          "DELETE FROM public.%1 WHERE id = (SELECT id FROM public.%1 WHERE ")
          .arg(tableName);
  bool flag = false;
  for (QMap<QString, QString>::const_iterator it = condition.constBegin();
       it != condition.constEnd(); it++) {
    if (it.value().size() > 0) {
      flag = true;
      requestText += QString("%1 = '%2' AND ").arg(it.key(), it.value());
    }
  }
  if (flag) {
    requestText.chop(4);
  }
  requestText += "ORDER BY id DESC LIMIT 1);";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (request.exec(requestText)) {
    sendLog("Последняя запись удалена. ");
    return true;
  } else {  // Обработка ошибки выполнения запроса
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }
}

void PostgresController::loadSettings() {
  // Загружаем настройки
  QSettings settings;

  HostAddress = settings.value("Database/Server/Ip").toString();
  Port = settings.value("Database/Server/Port").toInt();
  DatabaseName = settings.value("Database/Name").toString();
  UserName = settings.value("Database/User/Name").toString();
  Password = settings.value("Database/User/Password").toString();
  LogOption = settings.value("Database/Log/Active").toBool();
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
  for (int32_t i = 0; i < request.record().count(); i++) {
    record.insert(request.record().fieldName(i), request.value(i).toString());
  }
}
