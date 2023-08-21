#include "database_buffer.h"

DatabaseBuffer::DatabaseBuffer(QObject* parent) : QAbstractTableModel(parent) {
  setObjectName("DatabaseBuffer");

  Headers = nullptr;
  Data = nullptr;
}

DatabaseBuffer::~DatabaseBuffer() {
  clear();
}

void DatabaseBuffer::build(QVector<QString>* headers,
                           QVector<QVector<QString>*>* data) {
  // Проверка на существование
  if ((!headers) || (!data)) {
    return;
  }

  // Очищаем старые данные
  clear();

  beginResetModel();

  // Устанавливаем новые данные
  Headers = headers;
  Data = data;

  endResetModel();
}

void DatabaseBuffer::clear() {
  if ((!Headers) || (!Data))
    return;

  beginResetModel();

  delete Headers;
  Headers = nullptr;

  for (int32_t i = 0; i < Data->size(); i++)
    delete Data->at(i);

  delete Data;
  Data = nullptr;

  endResetModel();
}

int DatabaseBuffer::rowCount(const QModelIndex& parent) const {
  if (Data)
    return Data->size();
  else
    return 0;
}

int DatabaseBuffer::columnCount(const QModelIndex& parent) const {
  if (Headers)
    return Headers->size();
  else
    return 0;
}

QVariant DatabaseBuffer::data(const QModelIndex& index, int role) const {
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

QVariant DatabaseBuffer::headerData(int section,
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
