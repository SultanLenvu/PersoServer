#include "table_record.h"

TableRecord::TableRecord(QObject* parent) : QObject(parent) {}

ProductionLineRecord::ProductionLineRecord(QObject* parent)
    : TableRecord(parent) {}

TransponderRecord::TransponderRecord(QObject* parent) : TableRecord(parent) {}

OrderRecord::OrderRecord(QObject* parent) : TableRecord(parent) {}

IssuerRecord::IssuerRecord(QObject* parent) : TableRecord(parent) {}

BoxRecord::BoxRecord(QObject* parent) : TableRecord(parent) {}

PalleteRecord::PalleteRecord(QObject* parent) : TableRecord(parent) {}

TransportMasterKeyRecord::TransportMasterKeyRecord(QObject* parent)
    : TableRecord(parent) {}

CommercialMasterKeyRecord::CommercialMasterKeyRecord(QObject* parent)
    : TableRecord(parent) {}
