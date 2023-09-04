#include "firmware_generation_system.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(QObject *parent) : QObject(parent)
{
  setObjectName("FirmwareGenerationSystem");

  FirmwareBase = new QFile();
  FirmwareData = new QFile();

  loadSettings();
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
}

bool FirmwareGenerationSystem::getCurrentTransponderData(
    TransponderInfoModel* model) {}

bool FirmwareGenerationSystem::getAssembledFirmware(QMap<QString, QString> seed,
                                                    QByteArray* firmware) {}

void FirmwareGenerationSystem::loadSettings() {
  QSettings settings;

  FirmwareBase->setFileName(settings.value("Firmware/Base/Path").toString());
  FirmwareData->setFileName(settings.value("Firmware/Data/Path").toString());
}

void FirmwareGenerationSystem::generateFirmwareData() {}

void FirmwareGenerationSystem::assembleFirmware() {}
