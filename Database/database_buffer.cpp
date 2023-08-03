#include "database_buffer.h"

DatabaseBuffer::DatabaseBuffer(QObject* parent) : QAbstractTableModel(parent) {}

int DatabaseBuffer::rowCount(const QModelIndex& parent) const {
  return Buffer[0].size();
}

int DatabaseBuffer::columnCount(const QModelIndex& parent) const {
  return Buffer.size();
}

QVariant DatabaseBuffer::data(const QModelIndex& index, int role) const {
  if (index.row() > Buffer[0].size())
    return QVariant();

  if (index.column() > Buffer.size())
    return QVariant();

  if (role == Qt::DisplayRole) {
    return Buffer[index.row()][index.column()];
  }
}

QVariant DatabaseBuffer::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    if (section > Buffer.size())
      return QVariant();

    return QString("Значение");
  } else {
    if (section > Buffer[0].size())
      return QVariant();

    return Buffer[0][section];
  }
}
