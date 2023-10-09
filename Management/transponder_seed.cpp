#include "transponder_seed_model.h"

TransponderSeed::TransponderSeed(QObject* parent) : QObject(parent) {
  setObjectName("TransponderSeed");
  Attributes = nullptr;
  MasterKeys = nullptr;
}

TransponderSeed::~TransponderSeed() {
  deleteAll();
}

void TransponderSeed::build(const QMap<QString, QString>* attributes,
                            const QMap<QString, QString>* masterKeys) {
  // Проверка на существование
  if ((!attributes) || (!masterKeys)) {
    return;
  }

  // Очищаем старые данные
  deleteAll();

  // Устанавливаем новые данные
  Attributes = attributes;
  MasterKeys = masterKeys;
}

void TransponderSeed::clear() {
  deleteAll();
}

bool TransponderSeed::isEmpty() {
  if ((!Attributes) && (!MasterKeys)) {
    return true;
  }

  return false;
}

const QMap<QString, QString>* TransponderSeed::attributes() const {
  return Attributes;
}

const QMap<QString, QString>* TransponderSeed::masterKeys() const {
  return MasterKeys;
}

void TransponderSeed::deleteAll() {
  delete Attributes;
  Attributes = nullptr;
  delete MasterKeys;
  MasterKeys = nullptr;
}
