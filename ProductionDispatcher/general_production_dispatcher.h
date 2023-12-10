#ifndef GENERALPRODUCTIONDISPATCHER_H
#define GENERALPRODUCTIONDISPATCHER_H

#include "Database/abstract_sql_database.h"
#include "abstract_firmware_generation_system.h"
#include "abstract_info_system.h"
#include "abstract_launch_system.h"
#include "abstract_production_dispatcher.h"
#include "abstract_sticker_printer.h"
#include "abstract_release_system.h"

class GeneralProductionDispatcher : public AbstractProductionDispatcher {
  Q_OBJECT
 private:
  QHash<QString, std::unique_ptr<ProductionContext>> Contexts;

  size_t CheckPeriod;
  std::unique_ptr<QTimer> CheckTimer;
  std::shared_ptr<AbstractSqlDatabase> Database;

  std::unique_ptr<AbstractInfoSystem> Informer;
  std::unique_ptr<AbstractLaunchSystem> Launcher;
  std::unique_ptr<AbstractReleaseSystem> Releaser;
  std::unique_ptr<AbstractFirmwareGenerationSystem> Generator;

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
  ~GeneralProductionDispatcher();

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
  void instanceThreadStarted_slot(void);

  void loadSettings(void);
  void sendLog(const QString& log);

  void createLaunchSystem(void);
  void createReleaseSystem(void);
  void createInfoSystem(void);
  void createStickerPrinters(void);
  void createDatabase(void);
  void createFirmwareGenerator(void);
  void createCheckTimer(void);

 private slots:
  void on_CheckTimerTemeout(void);
};

#endif  // GENERALPRODUCTIONDISPATCHER_H
