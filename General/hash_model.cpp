#include "hash_model.h"

HashModel::HashModel(QObject* parent) : QAbstractTableModel(parent) {
  setObjectName("HashModel");
  createMatchTables();
}

HashModel::~HashModel() {}

void HashModel::buildTransponderData(const QHash<QString, QString>* data) {
  // Проверка на существование
  if (!data) {
    return;
  }

  beginResetModel();

  // Очищаем старые данные
  Values.clear();
  Headers.clear();

  // Устанавливаем новые данные
  for (QHash<QString, QString>::const_iterator it1 = data->constBegin();
       it1 != data->constEnd(); it1++) {
    Values.append(it1.value());
    Headers.append(TransponderDataMatchTable.value(it1.key()));
    HashTable.insert(TransponderDataMatchTable.value(it1.key()), it1.value());
  }

  endResetModel();
}

void HashModel::clear() {
  beginResetModel();

  Values.clear();
  Headers.clear();

  endResetModel();
}

bool HashModel::isEmpty() const {
  return HashTable.isEmpty();
}

const QHash<QString, QVariant>* HashModel::hash() const {
  return &HashTable;
}

int HashModel::columnCount(const QModelIndex& parent) const {
  if (HashTable.isEmpty()) {
    return 0;
  }

  return 1;
}

int HashModel::rowCount(const QModelIndex& parent) const {
  return HashTable.size();
}

QVariant HashModel::data(const QModelIndex& index, int role) const {
  if (index.column() > 1) {
    return QVariant();
  }

  if (index.row() > (HashTable.size())) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return Values.at(index.row());
  } else
    return QVariant();
}

QVariant HashModel::headerData(int section,
                               Qt::Orientation orientation,
                               int role) const {
  if (section > (HashTable.size())) {
    return QVariant();
  }

  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    return "Значение";
  }

  if (orientation == Qt::Vertical) {
    return Headers.at(section);
  } else {
    return QVariant();
  }
}

void HashModel::createMatchTables() {
  TransponderDataMatchTable.insert("sn", "Серийный номер");
  TransponderDataMatchTable.insert("pan", "PAN");
  TransponderDataMatchTable.insert("issuer_name", "Компания-заказчик");
  TransponderDataMatchTable.insert("box_id", "Идентификатор бокса");
  TransponderDataMatchTable.insert("pallet_id", "Идентификатор паллеты");
  TransponderDataMatchTable.insert("order_id", "Идентификатор заказа");
  TransponderDataMatchTable.insert("transponder_model", "Модель транспондера");
}
