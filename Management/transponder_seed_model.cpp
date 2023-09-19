#include "transponder_seed_model.h"

TransponderSeedModel::TransponderSeedModel(QObject* parent)
    : QAbstractTableModel(parent) {
  setObjectName("TransponderSeedModel");
  Attributes = nullptr;
  MasterKeys = nullptr;
}

TransponderSeedModel::~TransponderSeedModel() {
  deleteAll();
}

void TransponderSeedModel::build(const QMap<QString, QString>* attributes,
                                 const QMap<QString, QString>* masterKeys) {
  // Проверка на существование
  if ((!attributes) || (!masterKeys)) {
    return;
  }

  beginResetModel();

  // Очищаем старые данные
  deleteAll();

  // Устанавливаем новые данные
  Attributes = attributes;
  MasterKeys = masterKeys;

  endResetModel();
}

void TransponderSeedModel::clear() {
  beginResetModel();

  deleteAll();

  endResetModel();
}

bool TransponderSeedModel::isEmpty() {
  if ((!Attributes) && (!MasterKeys)) {
    return true;
  }

  return false;
}

const QMap<QString, QString>* TransponderSeedModel::attributes() const {
  return Attributes;
}

const QMap<QString, QString>* TransponderSeedModel::masterKeys() const {
  return MasterKeys;
}

int TransponderSeedModel::columnCount(const QModelIndex& parent) const {
  if ((!Attributes) || (!MasterKeys)) {
    return 0;
  }

  return 1;
}

int TransponderSeedModel::rowCount(const QModelIndex& parent) const {
  if ((!Attributes) || (!MasterKeys)) {
    return 0;
  }

  return Attributes->size() + MasterKeys->size();
}

QVariant TransponderSeedModel::data(const QModelIndex& index, int role) const {
  if ((!Attributes) || (!MasterKeys)) {
    return QVariant();
  }

  if (index.column() > 1) {
    return QVariant();
  }

  if (index.row() > (Attributes->size() + MasterKeys->size())) {
    return QVariant();
  }

  int32_t shiftSection = 0;
  if (role == Qt::DisplayRole) {
    if (index.row() < Attributes->size()) {
      return (Attributes->constBegin() + index.row()).value();
    } else {
      shiftSection = index.row() - Attributes->size();
      return (MasterKeys->constBegin() + shiftSection).value();
    }
  } else
    return QVariant();
}

QVariant TransponderSeedModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if ((!Attributes) || (!MasterKeys)) {
    return QVariant();
  }

  if (section > (Attributes->size() + MasterKeys->size())) {
    return QVariant();
  }

  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    return "Значение";
  }

  int32_t shiftSection = 0;
  if (orientation == Qt::Vertical) {
    if (section < Attributes->size()) {
      return (Attributes->constBegin() + section).key();
    } else {
      shiftSection = section - Attributes->size();
      return (MasterKeys->constBegin() + shiftSection).key();
    }
  } else {
    return QVariant();
  }
}

void TransponderSeedModel::deleteAll() {
  delete Attributes;
  Attributes = nullptr;
  delete MasterKeys;
  MasterKeys = nullptr;
}
