#include "abstract_firmware_generation_system.h"
#include "global_environment.h"
#include "log_system.h"

AbstractFirmwareGenerationSystem::AbstractFirmwareGenerationSystem(
    const QString& name) {
  setObjectName(name);

  connect(this, &AbstractFirmwareGenerationSystem::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractFirmwareGenerationSystem::~AbstractFirmwareGenerationSystem() {}

AbstractFirmwareGenerationSystem::AbstractFirmwareGenerationSystem()
    : QObject{nullptr} {}
