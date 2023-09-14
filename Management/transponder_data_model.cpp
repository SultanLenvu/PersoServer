#include "transponder_data_model.h"

TransponderSeedModel::TransponderSeedModel(QObject* parent)
    : QAbstractTableModel(parent) {
  setObjectName("TransponderSeedModel");
  Data = nullptr;
}

TransponderSeedModel::~TransponderSeedModel() {
  deleteAll();
}

void TransponderSeedModel::build(const QMap<QString, QString>* data) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  // Проверка на существование
  if (!data) {
    return;
  }

  beginResetModel();

  // Очищаем старые данные
  deleteAll();

  // Устанавливаем новые данные
  Data = data;

  endResetModel();
}

void TransponderSeedModel::clear() {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  beginResetModel();

  deleteAll();

  endResetModel();
}

bool TransponderSeedModel::isEmpty() {
  return (!Data) ? true : false;
}

const QMap<QString, QString>* TransponderSeedModel::data() {
  return Data;
}

int TransponderSeedModel::columnCount(const QModelIndex& parent) const {
  return 1;
}

int TransponderSeedModel::rowCount(const QModelIndex& parent) const {
  return (Data) ? Data->size() : 0;
}

QVariant TransponderSeedModel::data(const QModelIndex& index, int role) const {
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

QVariant TransponderSeedModel::headerData(int section,
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

void TransponderSeedModel::deleteAll() {
  delete Data;
  Data = nullptr;
}
