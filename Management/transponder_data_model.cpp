#include "transponder_data_model.h"

TransponderDataModel::TransponderDataModel(QObject* parent)
    : QAbstractTableModel(parent) {
  setObjectName("TransponderDataModel");
  Data = nullptr;
}

void TransponderDataModel::build(QMap<QString, QString>* data) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  // Проверка на существование
  if (!data) {
    return;
  }

  beginResetModel();

  // Очищаем старые данные
  deleteData();

  // Устанавливаем новые данные
  Data = data;

  endResetModel();
}

void TransponderDataModel::clear() {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  beginResetModel();

  deleteData();

  endResetModel();
}

bool TransponderDataModel::isEmpty() {
  return (!Data) ? true : false;
}

const QMap<QString, QString>* TransponderDataModel::getMap() {
  return Data;
}

int TransponderDataModel::columnCount(const QModelIndex& parent) const {
  return 1;
}

int TransponderDataModel::rowCount(const QModelIndex& parent) const {
  return (Data) ? Data->size() : 0;
}

QVariant TransponderDataModel::data(const QModelIndex& index, int role) const {
  if (!Data) {
    return QVariant();
  }

  if (index.column() > 1) {
    return QVariant();
  }

  if (index.row() > Data->size()) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return (Data->begin() + index.row()).value();
  } else
    return QVariant();
}

QVariant TransponderDataModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (!Data) {
    return QVariant();
  }

  if (section > Data->size()) {
    return QVariant();
  }

  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    return "Значение";
  }

  if (orientation == Qt::Vertical) {
    return (Data->begin() + section).key();
  } else {
    return QVariant();
  }
}

void TransponderDataModel::deleteData() {
  if (!Data)
    return;

  Data->clear();

  delete Data;
  Data = nullptr;
}
