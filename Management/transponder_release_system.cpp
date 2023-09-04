#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(
    QWidget* parent,
    IDatabaseController* database)
    : QWidget(parent) {
  setObjectName("TransponderReleaseSystem");
  Database = database;
}

void TransponderReleaseSystem::confirmTransponderRelease() {}

void TransponderReleaseSystem::loadSettings() {}
