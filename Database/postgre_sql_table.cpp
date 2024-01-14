#include "postgre_sql_table.h"

PostgreSqlTable::PostgreSqlTable(const QString& name,
                                 const QString& connectionName)
    : AbstractSqlTable(name) {
  ConnectionName = connectionName;

  loadSettings();

  RecordMaxCount = 1000;
  CurrentOrder = "ASC";
}

PostgreSqlTable::~PostgreSqlTable() {}

QString PostgreSqlTable::getPrimaryKey() const {
  return PrimaryKey;
}

void PostgreSqlTable::setPrimaryKey(const QString& newPrimaryKey) {
  PrimaryKey = newPrimaryKey;
}

uint32_t PostgreSqlTable::getRecordMaxCount() const {
  return RecordMaxCount;
}

void PostgreSqlTable::setRecordMaxCount(uint32_t newRecordMaxCount) {
  RecordMaxCount = newRecordMaxCount;
}

Qt::SortOrder PostgreSqlTable::getCurrentOrder() const {
  if (CurrentOrder == "ASC") {
    return Qt::AscendingOrder;
  }

  return Qt::DescendingOrder;
}

void PostgreSqlTable::setCurrentOrder(Qt::SortOrder order) {
  if (order == Qt::AscendingOrder) {
    CurrentOrder = "ASC";
    return;
  }

  CurrentOrder = "DESC";
}

const QVector<QString>* PostgreSqlTable::columns() const {
  return &Columns;
}

const QHash<QString, QString>* PostgreSqlTable::relations() const {
  return &Relations;
}

bool PostgreSqlTable::init() {
  QSqlDatabase db = QSqlDatabase::database(ConnectionName);
  if (!db.isOpen()) {
    return false;
  }

  QSqlQuery request(db);
  QSqlRecord record = db.record(objectName());

  // Получаем названия колонок
  for (int32_t i = 0; i < record.count(); i++) {
    Columns.append(record.fieldName(i));
  }

  // Ищем первичный ключ таблицы
  if (!request.exec(QString("SELECT attname AS column_name "
                            "FROM pg_constraint AS con "
                            "JOIN pg_attribute AS att ON att.attnum = "
                            "ANY(con.conkey) AND att.attrelid = con.conrelid "
                            "WHERE "
                            "con.contype = 'p' "
                            "AND con.conrelid = '%1'::regclass;")
                        .arg(objectName()))) {
    sendLog("Получена ошибка при поиске первичного ключа.");
    return false;
  }
  request.next();
  PrimaryKey = request.record().value(0).toString();

  // Ищем зависимости таблицы
  if (!request.exec(
          QString("SELECT "
                  "attname AS column_name, "
                  "confrelid::regclass AS referenced_table "
                  "FROM pg_constraint AS con "
                  "JOIN pg_attribute AS att ON att.attnum = ANY(con.conkey) "
                  "AND att.attrelid = con.conrelid "
                  "JOIN pg_class AS rel ON con.confrelid = rel.oid "
                  "WHERE "
                  "con.contype = 'f' "
                  "AND con.conrelid = '%1'::regclass;")
              .arg(objectName()))) {
    sendLog("Получена ошибка при поиске внешних зависимостей.");
    return false;
  }
  while (request.next()) {
    Relations.insert(request.value(1).toString(), request.value(0).toString());
  }

  return true;
}

void PostgreSqlTable::applySettings() {
  sendLog("Применение новых настроек. ");
  loadSettings();
}

bool PostgreSqlTable::createRecords(const SqlQueryValues& records) {
  if (!checkFieldNames(records)) {
    return false;
  }

  // Создаем запрос
  QString requestText = QString("INSERT INTO public.%1 (").arg(objectName());
  for (int32_t i = 0; i < records.fieldCount(); i++) {
    requestText += QString("%1, ").arg(records.fieldName(i));
  }
  requestText.chop(2);

  requestText += QString(") VALUES ");
  records.appendToInsert(requestText);
  requestText += QString(";");

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  return true;
}

bool PostgreSqlTable::readRecords(SqlQueryValues& response) {
  // Создаем запрос
  QString requestText = QString("SELECT * FROM public.%1 ORDER BY %2 %3 ")
                            .arg(objectName(), PrimaryKey, CurrentOrder);
  if (RecordMaxCount > 0) {
    requestText += QString("LIMIT %1").arg(QString::number(RecordMaxCount));
  }
  requestText += ";";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  response.extractRecords(request);
  return true;
}

bool PostgreSqlTable::readRecords(const QString& conditions,
                                  SqlQueryValues& response) {
  // Создаем запрос
  QString requestText =
      QString("SELECT * FROM public.%1 WHERE %2 ORDER BY %3 %4 ")
          .arg(objectName(), conditions, PrimaryKey, CurrentOrder);
  if (RecordMaxCount > 0) {
    requestText += QString("LIMIT %1").arg(QString::number(RecordMaxCount));
  }
  requestText += ";";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  response.extractRecords(request);
  return true;
}

bool PostgreSqlTable::readLastRecord(SqlQueryValues& response) {
  // Создаем запрос
  QString requestText =
      QString("SELECT * FROM public.%1 ORDER BY %2 ASC LIMIT 1;")
          .arg(objectName(), PrimaryKey);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  response.extractRecords(request);
  return true;
}

bool PostgreSqlTable::updateRecords(const SqlQueryValues& newValues) {
  if (!checkFieldNames(newValues)) {
    sendLog("Получено неизвестное имя поля таблицы. ");
    return false;
  }

  // Создаем запрос
  QString requestText = QString("UPDATE public.%1 SET ").arg(objectName());
  for (int32_t i = 0; i < newValues.fieldCount(); i++) {
    if (newValues.get(i) == "NULL") {
      requestText += QString("%1 = NULL, ").arg(newValues.fieldName(i));
    } else {
      requestText +=
          QString("%1 = '%2', ").arg(newValues.fieldName(i), newValues.get(i));
    }
  }
  requestText.chop(2);
  requestText += ";";

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  return true;
}

bool PostgreSqlTable::updateRecords(const QString& condition,
                                    const SqlQueryValues& newValues) {
  if (!checkFieldNames(newValues)) {
    sendLog("Получено неизвестное имя поля таблицы. ");
    return false;
  }

  // Создаем запрос
  QString requestText = QString("UPDATE public.%1 SET ").arg(objectName());
  for (int32_t i = 0; i < newValues.fieldCount(); i++) {
    if (newValues.get(i) == "NULL") {
      requestText += QString("%1 = NULL, ").arg(newValues.fieldName(i));
    } else {
      requestText +=
          QString("%1 = '%2', ").arg(newValues.fieldName(i), newValues.get(i));
    }
  }

  requestText.chop(2);
  requestText += QString(" WHERE %1;").arg(condition);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  return true;
}

bool PostgreSqlTable::deleteRecords(const QString& condition) {
  // Создаем запрос
  QString requestText =
      QString("DELETE FROM public.%1 WHERE %2;").arg(objectName(), condition);

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  return true;
}

bool PostgreSqlTable::clear() {
  // Создаем запрос
  QString requestText = QString("DELETE FROM public.%1;").arg(objectName());

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  return true;
}

bool PostgreSqlTable::getRecordCount(uint32_t& count) {
  // Создаем запрос
  QString requestText =
      QString("SELECT COUNT(*) FROM public.%1;").arg(objectName());

  // Выполняем запрос
  QSqlQuery request(QSqlDatabase::database(ConnectionName));
  request.setForwardOnly(true);
  if (!request.exec(requestText)) {
    sendLog(request.lastError().text());
    sendLog("Отправленный запрос: " + requestText);
    return false;
  }

  request.next();
  count = request.value(0).toUInt();

  return true;
}

void PostgreSqlTable::sendLog(const QString& log) {
  emit logging(QString("Table %1 - %2").arg(objectName(), log));
}

void PostgreSqlTable::loadSettings() {}

bool PostgreSqlTable::checkFieldNames(const SqlQueryValues& records) const {
  for (int32_t i = 0; i < records.fieldCount(); i++) {
    if (!Columns.contains(records.fieldName(i))) {
      return false;
    }
  }

  return true;
}

bool PostgreSqlTable::checkFieldNames(
    const QHash<QString, QString>& record) const {
  for (QHash<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    if (!Columns.contains(it.key())) {
      return false;
    }
  }

  return true;
}
