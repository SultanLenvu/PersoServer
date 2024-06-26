#ifndef TE310PRINTER_H
#define TE310PRINTER_H

#include <QHostInfo>

#include "General/definitions.h"
#include "isticker_printer.h"

class TE310Printer : public IStickerPrinter {
  Q_OBJECT

 private:
  // Прототипы библиотечных функции
  typedef int (*TscAbout)(void);
  typedef int (*TscOpenPort)(const char*);
  typedef int (*TscSendCommand)(const char*);
  typedef int (*TscClosePort)(void);

 private:
  QString Name;

  bool LibError;
  QString TscLibPath;
  QLibrary* TscLib;

  QHash<QString, QString> LastTransponderSticker;

  // Библиотечные функции
  TscAbout about;
  TscOpenPort openPort;
  TscSendCommand sendCommand;
  TscClosePort closePort;

 public:
  explicit TE310Printer(QObject* parent, const QString& name);

  virtual bool checkConfiguration(void) override;
  virtual ReturnStatus printTransponderSticker(
      const QHash<QString, QString>* parameters) override;
  virtual ReturnStatus printLastTransponderSticker(void) override;
  virtual ReturnStatus printBoxSticker(
      const QHash<QString, QString>* parameters) override;
  virtual ReturnStatus printPalletSticker(
      const QHash<QString, QString>* parameters) override;

  virtual ReturnStatus exec(const QStringList* commandScript) override;

  virtual void applySetting(void) override;

 private:
  Q_DISABLE_COPY_MOVE(TE310Printer);
  void loadSetting(void);
  bool loadTscLib(void);
  void printNkdSticker(const QHash<QString, QString>* parameters);
  void printZsdSticker(const QHash<QString, QString>* parameters);
};

#endif  // TE310PRINTER_H
