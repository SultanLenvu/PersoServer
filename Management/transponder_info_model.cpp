#include "transponder_info_model.h"

TransponderInfoModel::TransponderInfoModel(QObject* parent)
    : QAbstractTableModel(parent) {
  setObjectName("TransponderInfoModel");
  Data = nullptr;
}

void TransponderInfoModel::build(QMap<QString, QString>* data) {
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

void TransponderInfoModel::clear() {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  beginResetModel();

  deleteData();

  endResetModel();
}

bool TransponderInfoModel::isEmpty() {
  return (!Data) ? true : false;
}

const QMap<QString, QString>* TransponderInfoModel::getMap() {
  return Data;
}

int TransponderInfoModel::columnCount(const QModelIndex& parent) const {
  return 1;
}

int TransponderInfoModel::rowCount(const QModelIndex& parent) const {
  return (Data) ? Data->size() : 0;
}

QVariant TransponderInfoModel::data(const QModelIndex& index, int role) const {
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

QVariant TransponderInfoModel::headerData(int section,
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

void TransponderInfoModel::deleteData() {
  if (!Data)
    return;

  Data->clear();

  delete Data;
  Data = nullptr;
}
