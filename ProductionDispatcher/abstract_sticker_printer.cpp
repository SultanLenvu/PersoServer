#include "abstract_sticker_printer.h"

AbstractStickerPrinter::AbstractStickerPrinter(StickerPrinterType type)
    : QObject(nullptr) {
  setObjectName("AbstractStickerPrinter");
  Type = type;
}

AbstractStickerPrinter::~AbstractStickerPrinter() {}

void AbstractStickerPrinter::sendLog(const QString& log) {
  if (LogEnable) {
    emit logging(QString("%1 - %2").arg(objectName(), log));
  }
}
