#include <QSqlRecord>

#include "sql_query_values.h"

SqlQueryValues::SqlQueryValues() : QAbstractTableModel{nullptr} {}

SqlQueryValues::~SqlQueryValues() {}

QString SqlQueryValues::fieldName(uint32_t i) const {
  return Fields.at(i);
}

QString SqlQueryValues::get(uint32_t record, const QString& field) const {
  if (FieldIndex.contains(field)) {
    return QString();
  }

  if (Values.at(FieldIndex.value(field))->size() <= record) {
    return QString();
  }

  return Values.at(FieldIndex.value(field))->at(record);
}

QString SqlQueryValues::get(uint32_t record, uint32_t field) const {
  if (Values.size() <= field) {
    return QString();
  }

  if (Values.at(field)->size() <= record) {
    return QString();
  }

  return Values.at(field)->at(record);
}

QString SqlQueryValues::get(uint32_t field) const {
  if (Values.size() <= field) {
    return QString();
  }

  return Values.at(field)->at(0);
}

QString SqlQueryValues::get(const QString& field) const {
  if (FieldIndex.contains(field)) {
    return QString();
  }

  return Values.at(FieldIndex.value(field))->at(0);
}

QString SqlQueryValues::getLast(const QString& field) const {
  if (FieldIndex.contains(field)) {
    return QString();
  }

  return Values.at(FieldIndex.value(field))->last();
}

int32_t SqlQueryValues::recordCount() const {
  if (Values.isEmpty()) {
    return 0;
  }
  return Values.first()->size();
}

int32_t SqlQueryValues::fieldCount() const {
  return Fields.size();
}

bool SqlQueryValues::isEmpty() const {
  if (Values.isEmpty()) {
    return true;
  }
  return Values.first()->isEmpty();
}

void SqlQueryValues::appendToInsert(QString& queryText) const {
  for (int32_t i = 0; i < Values.first()->size(); i++) {
    queryText.append("(");
    for (int32_t j = 0; j < Values.size(); j++) {
      if (Values.at(j)->at(i) != "NULL") {
        queryText.append(QString("'%1'").arg(Values.at(j)->at(i)));
      } else {
        queryText.append(QString("NULL"));
      }
      queryText.append(", ");
    }
    queryText.chop(2);
    queryText.append("),\n");
  }
  queryText.chop(2);
}

void SqlQueryValues::extractRecords(QSqlQuery& request) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  beginResetModel();
  Values.clear();
  Fields.clear();
  FieldIndex.clear();

  QSqlRecord rec = request.record();
  for (int32_t i = 0; i < rec.count(); i++) {
    SharedVector<QString> values(new QVector<QString>());
    values->reserve(request.size());
    Values.append(values);

    Fields.append(rec.fieldName(i));
    FieldIndex.insert(rec.fieldName(i), i);
  }

  while (request.next()) {
    for (int32_t i = 0; i < request.record().count(); i++) {
      Values[i]->append(request.value(i).toString());
    }
  }

  endResetModel();
}

void SqlQueryValues::add(const QHash<QString, QString>& record) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  for (QHash<QString, QString>::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    add(it.key(), it.value());
  }
}

void SqlQueryValues::add(const QString& field,
                         const std::shared_ptr<QVector<QString>>& values) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  if (FieldIndex.contains(field)) {
    Values.at(FieldIndex.value(field))->append(*values);
  } else {
    Values.append(values);
    Fields.append(field);
    FieldIndex.insert(field, FieldIndex.size());
  }
}

void SqlQueryValues::add(const QString& field, const QString& value) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  if (FieldIndex.contains(field)) {
    Values.at(FieldIndex.value(field))->append(value);
  } else {
    std::shared_ptr<QVector<QString>> values(new QVector<QString>);
    values->append(value);
    FieldIndex.insert(field, FieldIndex.size());
    Values.append(values);
    Fields.append(field);
  }
}

void SqlQueryValues::addField(const QString& field) {
  if (!FieldIndex.contains(field)) {
    std::shared_ptr<QVector<QString>> values(new QVector<QString>);
    FieldIndex.insert(field, FieldIndex.size());
    Values.append(values);
    Fields.append(field);
  }
}

void SqlQueryValues::clear() {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  Values.clear();
  Fields.clear();
  FieldIndex.clear();
}

int SqlQueryValues::rowCount(const QModelIndex& parent) const {
  if ((Values.size() == 0) || (!Values.first())) {
    return 0;
  }

  return Values.first()->size();
}

int SqlQueryValues::columnCount(const QModelIndex& parent) const {
  return Fields.size();
}

QVariant SqlQueryValues::data(const QModelIndex& index, int role) const {
  if (index.column() > Fields.size())
    return QVariant();

  if (index.row() > Values.first()->size())
    return QVariant();

  if (role == Qt::DisplayRole) {
    return Values.at(index.column())->at(index.row());
  } else
    return QVariant();
}

QVariant SqlQueryValues::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const {
  if (section > Fields.size()) {
    return QVariant();
  }

  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    return Fields.at(section);
  } else {
    return QVariant();
  }
}
