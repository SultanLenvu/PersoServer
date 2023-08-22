#include "obu_issuer_order.h"

ObuIssuerOrder::ObuIssuerOrder(QObject* parent,
                               const QString& issuerName,
                               uint32_t obuQuantity)
    : QObject(parent) {
  PanFile = nullptr;
  IssuerName = issuerName;
  ObuQuantity = obuQuantity;

  FullPersonalization = false;
  ProductionStartDate = QDate::currentDate();
}

bool ObuIssuerOrder::setFullPersonalization(const QString& panFilePath) {
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

QString* ObuIssuerOrder::issuerName() {
  return &IssuerName;
}

bool ObuIssuerOrder::fullPersonalization() const {
  return FullPersonalization;
}

uint32_t ObuIssuerOrder::obuQuantity() const {
  return ObuQuantity;
}

QDate* ObuIssuerOrder::productionStartDate() {
  return &ProductionStartDate;
}

QString* ObuIssuerOrder::currentPan() {
  return &CurrentPan;
}

bool ObuIssuerOrder::nextPan() {
  if (!PanFile) {
    return false;
  }

  if (PanFile->atEnd()) {
    return false;
  }

  CurrentPan = PanFile->readLine();

  return true;
}
