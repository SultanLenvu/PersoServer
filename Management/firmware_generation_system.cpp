#include "firmware_generation_system.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(QObject *parent) : QObject(parent)
{
  setObjectName("FirmwareGenerationSystem");

  FirmwareBase = new QFile(this);
  FirmwareData = new QFile(this);

  loadSettings();
}

void FirmwareGenerationSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
}

bool FirmwareGenerationSystem::generate(TransponderSeedModel* seed,
                                        QByteArray* firmware) {}

void FirmwareGenerationSystem::loadSettings() {
  QSettings settings;

  FirmwareBase->setFileName(settings.value("Firmware/Base/Path").toString());
  FirmwareData->setFileName(settings.value("Firmware/Data/Path").toString());
}

void FirmwareGenerationSystem::generateCommonKeys(TransponderSeedModel* seed) {}

void FirmwareGenerationSystem::generateFirmwareData() {}

void FirmwareGenerationSystem::assembleFirmware() {}
