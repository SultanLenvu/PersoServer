#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(
    QObject* parent,
    IDatabaseController* database)
    : QObject(parent) {
  setObjectName("TransponderReleaseSystem");
  Database = database;

  loadSettings();
}

bool TransponderReleaseSystem::getSeed(const TransponderInfoModel& seed) {
  return true;
}

bool TransponderReleaseSystem::confirmRelease(
    const TransponderInfoModel& seed) {
  return true;
}

bool TransponderReleaseSystem::refund(const TransponderInfoModel& seed) {
  return true;
}

void TransponderReleaseSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
}

void TransponderReleaseSystem::loadSettings() {}

void TransponderReleaseSystem::proxyLogging(const QString& log) const {
  if (sender()->objectName() == "IDatabaseController") {
    emit logging("Database controller - " + log);
  } else {
    emit logging("Unknown - " + log);
  }
}
