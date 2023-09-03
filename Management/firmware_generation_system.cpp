#include "firmware_generation_system.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(QObject *parent) : QObject(parent)
{
  
}

void FirmwareGenerationSystem::proxyLogging(const QString& log) {
  if (sender()->objectName() == "IDatabaseController") {
    emit logging("Database controller - " + log);
  } else {
    emit logging("Unknown - " + log);
  }
}

void FirmwareGenerationSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
  Database->applySettings();
}

void FirmwareGenerationSystem::getCurrentTransponderData(
    QMap<QString, QString> transponderData) {}

void FirmwareGenerationSystem::getGeneratedFirmware(QByteArray* firmware) {}

void FirmwareGenerationSystem::confirmTransponderEmission() {}

void FirmwareGenerationSystem::createDatabaseController() {}

void FirmwareGenerationSystem::loadSettings() {}

void FirmwareGenerationSystem::generateTransponderData() {}

void FirmwareGenerationSystem::generateFirmware() {}

void FirmwareGenerationSystem::processingResult(const QString& log,
                                                const ExecutionStatus status) {
  emit logging(log);
  emit logging("Отключение от базы данных. ");

  if (status == CompletedSuccessfully) {
    Database->disconnect(IDatabaseController::Complete);
  } else {
    Database->disconnect(IDatabaseController::Abort);
  }
  emit operationFinished(status);
}
