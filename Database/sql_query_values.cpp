#include <QSqlRecord>

#include "sql_query_values.h"

SqlQueryValues::SqlQueryValues() : QAbstractTableModel{nullptr} {}

SqlQueryValues::~SqlQueryValues() {}

SqlQueryValues::SqlQueryValues(const SqlQueryValues& other)
    : Fields(other.Fields),
      FieldIndex(other.FieldIndex),
      Values(other.Values) {}

SqlQueryValues::SqlQueryValues(SqlQueryValues&& other) noexcept
    : Fields(std::move(other.Fields)),
      FieldIndex(std::move(other.FieldIndex)),
      Values(std::move(other.Values)) {}

SqlQueryValues& SqlQueryValues::operator=(const SqlQueryValues& other) {
  if (this != &other) {
    Fields = other.Fields;
    FieldIndex = other.FieldIndex;
    Values = other.Values;
  }
  return *this;
}

SqlQueryValues& SqlQueryValues::operator=(SqlQueryValues&& other) noexcept {
  if (this != &other) {
    Fields = std::move(other.Fields);
    FieldIndex = std::move(other.FieldIndex);
    Values = std::move(other.Values);
  }
  return *this;
}

QString SqlQueryValues::fieldName(uint32_t i) const {
  return Fields.at(i);
}

QString SqlQueryValues::get(uint32_t record, const QString& field) const {
  if (!FieldIndex.contains(field)) {
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
  if (!FieldIndex.contains(field)) {
    return QString();
  }

  return Values.at(FieldIndex.value(field))->at(0);
}

QString SqlQueryValues::getLast(const QString& field) const {
  if (!FieldIndex.contains(field)) {
    return QString();
  }

  return Values.at(FieldIndex.value(field))->back();
}

StringDictionary SqlQueryValues::getRecord(uint32_t num) const {
  StringDictionary record;

  if (num >= Values.size()) {
    return record;
  }

  for (int32_t i = 0, s = Fields.size(); i < s; ++i) {
    record.insert(Fields[i], Values.at(i)->at(num));
  }

  return record;
}

size_t SqlQueryValues::recordCount() const {
  if (Values.empty()) {
    return 0;
  }
  return Values.front()->size();
}

size_t SqlQueryValues::fieldCount() const {
  return Fields.size();
}

bool SqlQueryValues::isEmpty() const {
  if (Values.empty()) {
    return true;
  }
  return Values.front()->empty();
}

void SqlQueryValues::appendToInsert(QString& queryText) const {
  for (int32_t i = 0; i < Values.front()->size(); i++) {
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
  beginResetModel();

  Values.clear();
  Fields.clear();
  FieldIndex.clear();

  // Проверяем наличие записей
  if (!request.last()) {
    return;
  }
  // ... и сохраняем их количество
  int32_t recCount = request.at() + 1;
  request.seek(QSql::BeforeFirstRow);

  // Сохраняем количество полей
  QSqlRecord rec = request.record();
  int32_t fieldCount = rec.count();
  Values.reserve(fieldCount);

  for (int32_t i = 0; i < fieldCount; ++i) {
    SharedVector<QString> values(new std::vector<QString>());
    values->reserve(recCount);
    Values.push_back(values);

    Fields.append(rec.fieldName(i));
    FieldIndex.insert(rec.fieldName(i), i);
  }

  while (request.next()) {
    for (int32_t i = 0; i < fieldCount; i++) {
      Values[i]->push_back(request.value(i).toString());
    }
  }

  endResetModel();
}

void SqlQueryValues::add(const StringDictionary& record) {
  for (StringDictionary::const_iterator it = record.constBegin();
       it != record.constEnd(); it++) {
    add(it.key(), it.value());
  }
}

void SqlQueryValues::add(const QString& field, const QString& value) {
  if (FieldIndex.contains(field)) {
    Values.at(FieldIndex.value(field))->push_back(value);
  } else {
    SharedVector<QString> values(new std::vector<QString>);
    values->push_back(value);
    FieldIndex.insert(field, FieldIndex.size());
    Values.push_back(values);
    Fields.append(field);
  }
}

void SqlQueryValues::addField(const QString& field) {
  if (!FieldIndex.contains(field)) {
    SharedVector<QString> values(new std::vector<QString>);
    FieldIndex.insert(field, FieldIndex.size());
    Values.push_back(values);
    Fields.append(field);
  }
}

void SqlQueryValues::addField(const QString& field,
                              const SharedVector<QString> values) {
  if (FieldIndex.contains(field)) {
    Values.at(FieldIndex.value(field))
        ->insert(Values.at(FieldIndex.value(field))->end(), values->begin(),
                 values->end());
  } else {
    Values.push_back(values);
    Fields.append(field);
    FieldIndex.insert(field, FieldIndex.size());
  }
}

void SqlQueryValues::addRecord(const StringDictionary record) {
  if (record.empty()) {
    return;
  }

  for (auto it1 = record.begin(), it2 = record.end(); it1 != it2; ++it1) {
    add(it1.key(), it2.value());
  }
}

void SqlQueryValues::clear() {
  Values.clear();
  Fields.clear();
  FieldIndex.clear();
}

int SqlQueryValues::rowCount(const QModelIndex& parent) const {
  if ((Values.size() == 0) || (!Values.front())) {
    return 0;
  }

  return static_cast<int32_t>(Values.front()->size());
}

int SqlQueryValues::columnCount(const QModelIndex& parent) const {
  return Fields.size();
}

QVariant SqlQueryValues::data(const QModelIndex& index, int role) const {
  if (index.column() > Fields.size())
    return QVariant();

  if (index.row() > Values.front()->size())
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
