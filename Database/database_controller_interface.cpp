#include "database_controller_interface.h"

DatabaseControllerInterface::DatabaseControllerInterface(QObject* parent)
    : QObject(parent) {
  setObjectName("DatabaseControllerInterface");
}
