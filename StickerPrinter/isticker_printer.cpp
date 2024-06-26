#include "isticker_printer.h"

IStickerPrinter::IStickerPrinter(QObject* parent, StickerPrinterType type)
    : QObject(parent) {
  setObjectName("IStickerPrinter");
  Type = type;
}

IStickerPrinter::~IStickerPrinter() {}

void IStickerPrinter::sendLog(const QString& log) {
  if (LogEnable) {
    emit logging(QString("%1 - %2").arg(objectName(), log));
  }
}
