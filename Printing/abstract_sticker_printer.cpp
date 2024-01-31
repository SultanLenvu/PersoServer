#include "abstract_sticker_printer.h"
#include "global_environment.h"
#include "log_system.h"

AbstractStickerPrinter::AbstractStickerPrinter(const QString& name)
    : QObject(nullptr) {
  setObjectName(name);

  connect(this, &AbstractStickerPrinter::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

AbstractStickerPrinter::~AbstractStickerPrinter() {}
