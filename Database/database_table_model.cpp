#include "database_table_model.h"

DatabaseTableModel::DatabaseTableModel(QObject* parent)
    : QAbstractTableModel(parent) {
  setObjectName("DatabaseTableModel");

  Headers = nullptr;
  Data = nullptr;
}

DatabaseTableModel::~DatabaseTableModel() {
  clear();
}

void DatabaseTableModel::build(QVector<QString>* headers,
                               QVector<QVector<QString>*>* data) {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  // Проверка на существование
  if ((!headers) || (!data)) {
    return;
  }

  beginResetModel();

  // Очищаем старые данные
  deleteAll();

  // Устанавливаем новые данные
  Headers = headers;
  Data = data;

  endResetModel();
}

void DatabaseTableModel::clear() {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  beginResetModel();

  deleteAll();

  endResetModel();
}

bool DatabaseTableModel::isEmpty() {
  // Блокируем доступ
  QMutexLocker locker(&Mutex);

  if ((!Headers) && (!Data)) {
    return true;
  } else {
    return false;
  }
}

int DatabaseTableModel::rowCount(const QModelIndex& parent) const {
  if (Data)
    return Data->size();
  else
    return 0;
}

int DatabaseTableModel::columnCount(const QModelIndex& parent) const {
  if (Headers)
    return Headers->size();
  else
    return 0;
}

QVariant DatabaseTableModel::data(const QModelIndex& index, int role) const {
  if ((!Headers) || (!Data))
    return QVariant();

  if (index.column() > Headers->size())
    return QVariant();

  if (index.row() > Data->size())
    return QVariant();

  if (role == Qt::DisplayRole) {
    return Data->at(index.row())->at(index.column());
  } else
    return QVariant();
}

QVariant DatabaseTableModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
  if (!Headers)
    return QVariant();

  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    return Headers->at(section);
  } else {
    return QVariant();
  }
}

void DatabaseTableModel::deleteAll() {
  if ((!Headers) || (!Data))
    return;

  delete Headers;
  Headers = nullptr;

  for (int32_t i = 0; i < Data->size(); i++)
    delete Data->at(i);

  delete Data;
  Data = nullptr;
}
