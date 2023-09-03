#ifndef FIRMWAREGENERATIONSYSTEM_H
#define FIRMWAREGENERATIONSYSTEM_H

#include <QFile>
#include <QObject>
#include <QString>

#include <Database/database_controller.h>
#include <Database/postgres_controller.h>

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
  PostgresController* Database;
  QMap<QString, QString> CurrentTransponderData;

  QFile* Base;
  QFile* DsrcData;

  QByteArray* Firmware;

 public:
  explicit FirmwareGenerationSystem(QObject* parent);

 public slots:
  void proxyLogging(const QString& log);
  void applySettings(void);

  void getCurrentTransponderData(QMap<QString, QString> transponderData);
  void getGeneratedFirmware(QByteArray* firmware);
  void confirmTransponderEmission(void);

 private:
  void createDatabaseController(void);
  void loadSettings(void);

  void generateTransponderData(void);
  void generateFirmware(void);

  void processingResult(const QString& log, const ExecutionStatus status);

 signals:
  void logging(const QString& log) const;
  void operationFinished(ExecutionStatus status);
};

#endif // FIRMWAREGENERATIONSYSTEM_H
