#include "transponder_info_model.h"

TransponderInfoModel::TransponderInfoModel(QObject* parent)
    : QAbstractListModel(parent) {
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
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  return (!Data) ? true : false;
}

int TransponderInfoModel::rowCount(const QModelIndex& parent) const {
  return (Data) ? Data->size() : 0;
}

QVariant TransponderInfoModel::data(const QModelIndex& index, int role) const {
  if (!Data) {
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
