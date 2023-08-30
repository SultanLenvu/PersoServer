#include "administration_system_builder.h"

void AdministrationSystem::init() {}

AdministrationSystemBuilder::AdministrationSystemBuilder() : QObject(nullptr) {
  // Пока никакие объекты не созданы
  Administrator = nullptr;
}

AdministrationSystem* AdministrationSystemBuilder::buildedObject() const {
  return Administrator;
}

void AdministrationSystemBuilder::build() {
  Administrator = new AdministrationSystem(nullptr);

  emit completed();
}
