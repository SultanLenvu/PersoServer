#ifndef ISTICKERPRINTER_H
#define ISTICKERPRINTER_H

#include <QLibrary>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QtPrintSupport/QPrinterInfo>

class IStickerPrinter : public QObject {
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
  IStickerPrinter(QObject* parent, StickerPrinterType type);
  virtual ~IStickerPrinter();

  virtual bool checkConfiguration(void) = 0;
  virtual ReturnStatus printTransponderSticker(
      const QMap<QString, QString>* parameters) = 0;
  virtual ReturnStatus printLastTransponderSticker(void) = 0;
  virtual ReturnStatus printBoxSticker(
      const QMap<QString, QString>* parameters) = 0;
  virtual ReturnStatus printPalletSticker(
      const QMap<QString, QString>* parameters) = 0;

  virtual ReturnStatus exec(const QStringList* commandScript) = 0;

  virtual void applySetting(void) = 0;

 private:
  Q_DISABLE_COPY(IStickerPrinter);

 signals:
  void logging(const QString& log);
};

#endif  // ISTICKERPRINTER_H
