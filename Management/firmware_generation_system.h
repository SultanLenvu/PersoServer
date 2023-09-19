#ifndef FIRMWAREGENERATIONSYSTEM_H
#define FIRMWAREGENERATIONSYSTEM_H

#include <QAbstractTableModel>
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QObject>
#include <QSettings>
#include <QString>

#include "General/definitions.h"
#include "Security/des.h"
#include "transponder_seed_model.h"

class FirmwareGenerationSystem : public QObject
{
  Q_OBJECT
 public:
  enum ExecutionStatus {
    NotExecuted,
    GenerationError,
    DatabaseQueryError,
    UnknowError,
    CompletedSuccessfully
  };

 private:
  QFile* FirmwareBase;
  QFile* FirmwareData;

  QByteArray GeneratedFirmware;
  QMap<QString, QByteArray> CommonKeys;
  QMap<QString, QByteArray> MasterKeys;

 public:
  explicit FirmwareGenerationSystem(QObject* parent);
  void applySettings(void);

  bool generate(const TransponderSeedModel* seed, QByteArray* firmware);

 private:
  void loadSettings(void);

  void generateFirmwareData(void);
  void assembleFirmware(void);

  void generateCommonKeys(const TransponderSeedModel* seed);
  void generatePaymentMeans(const QString& pan, QString& paymentMeans);

 signals:
  void logging(const QString& log) const;
};

#endif // FIRMWAREGENERATIONSYSTEM_H
