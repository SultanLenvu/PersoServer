#include "issuer_order.h"

IssuerOrder::IssuerOrder(QObject* parent) : QObject(parent) {
  PanFile = nullptr;
  TransponderQuantity = 0;

  FullPersonalization = false;
  ProductionStartDate = QDate::currentDate();
}

void IssuerOrder::setIssuerName(const QString& newIssuerName) {
  IssuerName = newIssuerName;
}

void IssuerOrder::setTransponderQuantity(uint32_t newTransponderQuantity) {
  TransponderQuantity = newTransponderQuantity;
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

const QString& IssuerOrder::issuerName() {
  return IssuerName;
}

bool IssuerOrder::fullPersonalization() const {
  return FullPersonalization;
}

uint32_t IssuerOrder::transponderQuantity() const {
  return TransponderQuantity;
}

const QDate& IssuerOrder::productionStartDate() {
  return ProductionStartDate;
}

const QString& IssuerOrder::currentPan() {
  return CurrentPan;
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
