#include "abstract_firmware_generation_system.h"

AbstractFirmwareGenerationSystem::AbstractFirmwareGenerationSystem(
    const QString& name) {
  setObjectName(name);
}

AbstractFirmwareGenerationSystem::~AbstractFirmwareGenerationSystem() {}

AbstractFirmwareGenerationSystem::AbstractFirmwareGenerationSystem()
    : QObject{nullptr} {}
