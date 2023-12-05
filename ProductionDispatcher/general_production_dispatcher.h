#ifndef GENERALPRODUCTIONDISPATCHER_H
#define GENERALPRODUCTIONDISPATCHER_H

#include "abstract_production_dispatcher.h"
#include "abstract_sticker_printer.h"

class GeneralProductionDispatcher : public AbstractProductionDispatcher {
  Q_OBJECT
 private:
  bool LogEnable;

  QString BoxStickerPrinterName;
  QString PalletStickerPrinterName;
#ifdef __linux__
  QHostAddress BoxStickerPrinterIP;
  int32_t BoxStickerPrinterPort;
  QHostAddress PalletStickerPrinterIP;
  int32_t PalletStickerPrinterPort;
#endif /* __linux__ */

  std::unique_ptr<AbstractStickerPrinter> BoxStickerPrinter;
  std::unique_ptr<AbstractStickerPrinter> PalletStickerPrinter;

 public:
  explicit GeneralProductionDispatcher(const QString& name);

  virtual bool checkConfiguration(void) override;

  // AbstractProductionDispatcher interface
 public slots:
  virtual void start(ReturnStatus& ret) override;
  virtual void stop(void) override;
  virtual void launchProductionLine(const StringDictionary& param,
                                    StringDictionary& context,
                                    ReturnStatus& ret) override;
  virtual void shutdownProductionLine(const StringDictionary& param,
                                      ReturnStatus& ret) override;
  virtual void getProductionLineContext(const StringDictionary& param,
                                        StringDictionary& result,
                                        ReturnStatus& ret) override;
  virtual void rollbackProductionLine(const StringDictionary& param,
                                      StringDictionary& context,
                                      ReturnStatus& ret) override;
  virtual void releaseTransponder(const StringDictionary& param,
                                  StringDictionary& seed,
                                  ReturnStatus& ret) override;
  virtual void confirmTransponderRelease(const StringDictionary& param,
                                         StringDictionary& context,
                                         ReturnStatus& ret) override;
  virtual void rereleaseTransponder(const StringDictionary& param,
                                    StringDictionary& seed,
                                    ReturnStatus& ret) override;
  virtual void confirmTransponderRerelease(const StringDictionary& param,
                                           StringDictionary& context,
                                           ReturnStatus& ret) override;
  virtual void printBoxStickerManually(const StringDictionary& param,
                                       ReturnStatus& ret) override;
  virtual void printLastBoxStickerManually(ReturnStatus& ret) override;
  virtual void printPalletStickerManually(const StringDictionary& param,
                                          ReturnStatus& ret) override;
  virtual void printLastPalletStickerManually(ReturnStatus& ret) override;

 private:
  GeneralProductionDispatcher();
  Q_DISABLE_COPY_MOVE(GeneralProductionDispatcher);

  void loadSettings(void);
  void sendLog(const QString& log);

  void createStickerPrinters(void);
};

#endif  // GENERALPRODUCTIONDISPATCHER_H
