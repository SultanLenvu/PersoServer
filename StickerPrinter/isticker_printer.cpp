#include "isticker_printer.h"

IStickerPrinter::IStickerPrinter(QObject* parent, PrinterType type)
    : QObject(parent) {
  setObjectName("IStickerPrinter");
  Type = type;
}

IStickerPrinter::~IStickerPrinter() {}
