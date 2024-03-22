#ifndef TE310PRINTER_H
#define TE310PRINTER_H

#include <QHostAddress>
#include <QHostInfo>

#include "abstract_sticker_printer.h"

class TE310Printer : public AbstractStickerPrinter {
  Q_OBJECT

 private:
  // Прототипы библиотечных функции
  typedef int (*TscAbout)(void);
  typedef int (*TscOpenPort)(const char*);
  typedef int (*TscSendCommand)(const char*);
  typedef int (*TscClosePort)(void);
  typedef int (*TscOpenEthernet)(const char*, int);

 private:
  QString SystemName;

  bool UseEthernet;
  QHostAddress IPAddress;
  int32_t Port;

  QString TscLibPath;
  std::unique_ptr<QLibrary> TscLib;

  StringDictionary LastTransponderSticker;
  StringDictionary LastBoxSticker;
  StringDictionary LastPalletSticker;

  // Библиотечные функции
  TscAbout about;
  TscOpenPort openPort;
  TscSendCommand sendCommand;
  TscClosePort closePort;
  TscOpenEthernet openEthernet;

 public:
  explicit TE310Printer(const QString& name);

  virtual ReturnStatus checkConfig(void) override;
  virtual StickerPrinterType type(void) override;

  virtual ReturnStatus printTransponderSticker(
      const StringDictionary& param) override;
  virtual ReturnStatus printLastTransponderSticker(void) override;

  virtual ReturnStatus printBoxSticker(const StringDictionary& param) override;
  virtual ReturnStatus printLastBoxSticker(void) override;

  virtual ReturnStatus printPalletSticker(
      const StringDictionary& param) override;
  virtual ReturnStatus printLastPalletSticker(void) override;

  virtual ReturnStatus exec(const QStringList& commandScript) override;

  virtual void applySetting(void) override;

 private:
  Q_DISABLE_COPY_MOVE(TE310Printer);
  void loadSetting(void);
  void doLoadSettings(void);
  void sendLog(const QString& log);

  void loadTscLib(void);

  bool initConnection(void);

  void printNkdSticker(const StringDictionary& param);
  void printMssSticker(const StringDictionary& param);
};

#endif  // TE310PRINTER_H
