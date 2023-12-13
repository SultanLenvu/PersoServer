#include "abstract_sticker_printer.h"

AbstractStickerPrinter::AbstractStickerPrinter(StickerPrinterType type)
    : QObject(nullptr) {
  setObjectName("AbstractStickerPrinter");
  Type = type;
}

AbstractStickerPrinter::~AbstractStickerPrinter() {}
