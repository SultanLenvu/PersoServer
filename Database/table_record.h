#ifndef DATABASETABLERECORD_H
#define DATABASETABLERECORD_H

#include <QDate>
#include <QObject>
#include <QString>

class TableRecord : public QObject {
  Q_OBJECT
 public:
  explicit TableRecord(QObject* parent = nullptr);

 signals:
};

class ProductionLineRecord : public TableRecord {
  Q_OBJECT

 public:
  QString Login;
  QString Password;

 public:
  explicit ProductionLineRecord(QObject* parent = nullptr);

 signals:
};

class TransponderRecord : public TableRecord {
  Q_OBJECT
 public:
  uint32_t SerialNumber;
  QString Model;
  QByteArray UCID;
  QByteArray PaymentMeans;
  uint32_t EmissionCounter;
  uint32_t OrderId;
  uint32_t BoxId;

 public:
  explicit TransponderRecord(QObject* parent);

 signals:
};

class OrderRecord : public TableRecord {
  Q_OBJECT
 public:
  uint32_t Id;
  uint32_t TransponderQuantity;
  bool FullPersonalization;
  QDate StartProductionDate;
  QDate EndProductionDate;
  uint32_t IssuerId;

 public:
  explicit OrderRecord(QObject* parent);

 signals:
};

class IssuerRecord : public TableRecord {
  Q_OBJECT
 public:
  uint32_t Id;
  QString Name;
  QByteArray EfcContextMark;
  uint32_t OrderQuantity;

 public:
  explicit IssuerRecord(QObject* parent);

 signals:
};

class BoxRecord : public TableRecord {
  Q_OBJECT
 public:
  uint32_t Id;
  uint32_t TransponderQuantity;
  QString ProductionDuration;
  bool ReadyIndicator;
  uint32_t PalletId;

 public:
  explicit BoxRecord(QObject* parent);

 signals:
};

class PalleteRecord : public TableRecord {
  Q_OBJECT
 public:
  uint32_t Id;
  uint32_t BoxQuantity;
  QString ProductionDuration;
  bool ReadyIndicator;

 public:
  explicit PalleteRecord(QObject* parent);

 signals:
};

class TransportMasterKeyRecord : public TableRecord {
  Q_OBJECT
 public:
  QString AccrKey;
  QString PerKey;
  QString AuKey1;
  QString AuKey2;
  QString AuKey3;
  QString AuKey4;
  QString AuKey5;
  QString AuKey6;
  QString AuKey7;
  QString AuKey8;

 public:
  explicit TransportMasterKeyRecord(QObject* parent);

 signals:
};

class CommercialMasterKeyRecord : public TableRecord {
  Q_OBJECT
 public:
  QString AccrKey;
  QString PerKey;
  QString AuKey1;
  QString AuKey2;
  QString AuKey3;
  QString AuKey4;
  QString AuKey5;
  QString AuKey6;
  QString AuKey7;
  QString AuKey8;

 public:
  explicit CommercialMasterKeyRecord(QObject* parent);

 signals:
};

#endif  // DATABASETABLERECORD_H
