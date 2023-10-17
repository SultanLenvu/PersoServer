#include "isticker_printer.h"

IStickerPrinter::IStickerPrinter(QObject* parent, StickerPrinterType type)
    : QObject(parent) {
  setObjectName("IStickerPrinter");
  Type = type;
}

IStickerPrinter::~IStickerPrinter() {}
