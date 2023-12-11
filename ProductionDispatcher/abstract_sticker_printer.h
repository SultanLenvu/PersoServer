#ifndef ISTICKERPRINTER_H
#define ISTICKERPRINTER_H

#include <QHash>
#include <QLibrary>
#include <QObject>
#include <QSettings>
#include <QtPrintSupport/QPrinterInfo>

#include "General/types.h"

class AbstractStickerPrinter : public QObject {
  Q_OBJECT
 public:
  enum StickerPrinterType {
    Unknown,
    TE310,
  };
  Q_ENUM(StickerPrinterType);

  enum ReturnStatus {
    ParameterError,
    LibraryMissed,
    ConnectionError,
    Failed,
    Completed,
  };
  Q_ENUM(ReturnStatus);

 protected:
  StickerPrinterType Type;

 public:
  AbstractStickerPrinter(StickerPrinterType type);
  virtual ~AbstractStickerPrinter();

  virtual bool init(void) = 0;

  virtual ReturnStatus printTransponderSticker(
      const StringDictionary& param) = 0;
  virtual ReturnStatus printLastTransponderSticker(void) = 0;

  virtual ReturnStatus printBoxSticker(const StringDictionary& param) = 0;
  virtual ReturnStatus printLastBoxSticker(void) = 0;

  virtual ReturnStatus printPalletSticker(const StringDictionary& param) = 0;
  virtual ReturnStatus printLastPalletSticker(void) = 0;

  virtual ReturnStatus exec(const QStringList* commandScript) = 0;

  virtual void applySetting(void) = 0;

 private:
  Q_DISABLE_COPY_MOVE(AbstractStickerPrinter);

 signals:
};

#endif  // ISTICKERPRINTER_H
