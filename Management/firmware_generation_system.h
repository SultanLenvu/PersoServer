#ifndef FIRMWAREGENERATIONSYSTEM_H
#define FIRMWAREGENERATIONSYSTEM_H

#include <QAbstractTableModel>
#include <QFile>
#include <QObject>
#include <QSettings>
#include <QString>

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

  QByteArray* GeneratedFirmware;

 public:
  explicit FirmwareGenerationSystem(QObject* parent);
  void applySettings(void);

  bool generate(TransponderSeedModel* seed, QByteArray* firmware);

 private:
  void loadSettings(void);

  void generateFirmwareData(void);
  void assembleFirmware(void);

  void generateCommonKeys(TransponderSeedModel* seed);

 signals:
  void logging(const QString& log) const;
};

#endif // FIRMWAREGENERATIONSYSTEM_H
