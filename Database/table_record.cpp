#include "table_record.h"

TableRecord::TableRecord(QObject* parent) : QObject(parent) {}

ProductionLineRecord::ProductionLineRecord(QObject* parent)
    : TableRecord(parent) {}

TransponderRecord::TransponderRecord(QObject* parent) : TableRecord(parent) {}

OrderRecord::OrderRecord(QObject* parent) : TableRecord(parent) {}

const QString& OrderRecord::getId() const {
  return Id;
}

void OrderRecord::setId(uint32_t newId) {
  Id = newId;
}

const QString& OrderRecord::getFullPersonalization() const {
  return FullPersonalization;
}

void OrderRecord::setFullPersonalization(bool newFullPersonalization) {
  FullPersonalization =
      QString::fromStdString(newFullPersonalization ? "true" : "false");
}

const QString& OrderRecord::getProductionStartDate() const {
  return ProductionStartDate;
}

void OrderRecord::setProductionStartDate(const QDate& newProductionStartDate) {
  ProductionStartDate = newProductionStartDate.toString("dd.MM.yyyy");
}

const QString& OrderRecord::getProductionEndDate() const {
  return ProductionEndDate;
}

void OrderRecord::setProductionEndDate(const QDate& newProductionEndDate) {
  ProductionEndDate = newProductionEndDate.toString("dd.MM.yyyy");
}

const QString& OrderRecord::getIssuerId() const {
  return IssuerId;
}

void OrderRecord::setIssuerId(uint32_t newIssuerId) {
  IssuerId = QString::number(newIssuerId);
}

const QString& OrderRecord::getTransponderQuantity() const {
  return TransponderQuantity;
}

void OrderRecord::setTransponderQuantity(uint32_t newTransponderQuantity) {
  TransponderQuantity = QString::number(newTransponderQuantity);
}

IssuerRecord::IssuerRecord(QObject* parent) : TableRecord(parent) {}

BoxRecord::BoxRecord(QObject* parent) : TableRecord(parent) {}

PalletRecord::PalletRecord(QObject* parent) : TableRecord(parent) {}

TransportMasterKeyRecord::TransportMasterKeyRecord(QObject* parent)
    : TableRecord(parent) {}

CommercialMasterKeyRecord::CommercialMasterKeyRecord(QObject* parent)
    : TableRecord(parent) {}
