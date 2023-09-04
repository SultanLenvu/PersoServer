#ifndef FIRMWAREGENERATIONSYSTEM_H
#define FIRMWAREGENERATIONSYSTEM_H

#include <QAbstractTableModel>
#include <QFile>
#include <QObject>
#include <QString>

#include <Database/database_controller.h>
#include <Database/postgres_controller.h>
#include "transponder_info_model.h"

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

 public slots:
  void proxyLogging(const QString& log);
  void applySettings(void);

  bool getAssembledFirmware(TransponderInfoModel* seed, QByteArray* firmware);

 private:
  void loadSettings(void);

  void generateFirmwareData(void);
  void assembleFirmware(void);

 signals:
  void logging(const QString& log) const;
};

#endif // FIRMWAREGENERATIONSYSTEM_H
