#include "issuer_order.h"

IssuerOrder::IssuerOrder(const QString& issuerName,
                         uint32_t TransponderQuantity,
                         QObject* parent)
    : QObject(parent) {
  PanFile = nullptr;
  IssuerName = issuerName;
  TransponderQuantity = TransponderQuantity;

  FullPersonalization = false;
  ProductionStartDate = QDate::currentDate();
}

bool IssuerOrder::setFullPersonalization(const QString& panFilePath) {
  if (FullPersonalization) {
    return false;
  }

  FullPersonalization = true;

  PanFile = new QFile(panFilePath, this);

  if (!PanFile->open(QIODevice::ReadOnly)) {
    delete PanFile;
    return false;
  }

  return true;
}

QString* IssuerOrder::issuerName() {
  return &IssuerName;
}

bool IssuerOrder::fullPersonalization() const {
  return FullPersonalization;
}

uint32_t IssuerOrder::transponderQuantity() const {
  return TransponderQuantity;
}

QDate* IssuerOrder::productionStartDate() {
  return &ProductionStartDate;
}

QString* IssuerOrder::currentPan() {
  return &CurrentPan;
}

bool IssuerOrder::nextPan() {
  if (!PanFile) {
    return false;
  }

  if (PanFile->atEnd()) {
    return false;
  }

  CurrentPan = PanFile->readLine();

  return true;
}
