#ifndef PRODUCTIONDISPATCHER_H
#define PRODUCTIONDISPATCHER_H

#include "Database/abstract_sql_database.h"
#include "Printing/abstract_sticker_printer.h"
#include "abstract_firmware_generation_system.h"
#include "abstract_info_system.h"
#include "abstract_launch_system.h"
#include "abstract_production_dispatcher.h"
#include "abstract_release_system.h"

class ProductionDispatcher : public AbstractProductionDispatcher {
  Q_OBJECT
 private:
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

  std::shared_ptr<ProductionLineContext> Context;

 public:
  explicit ProductionDispatcher(const QString& name);
  ~ProductionDispatcher();

  // AbstractProductionDispatcher interface
 public slots:
  virtual void onInstanceThreadStarted(void) override;

  virtual void start(ReturnStatus& ret) override;
  virtual void stop(void) override;

  virtual void launchProductionLine(ReturnStatus& ret) override;
  virtual void shutdownProductionLine(ReturnStatus& ret) override;

  virtual void requestBox(ReturnStatus& ret) override;
  virtual void getCurrentBoxData(StringDictionary& result,
                                 ReturnStatus& ret) override;
  virtual void refundBox(ReturnStatus& ret) override;
  virtual void completeBox(ReturnStatus& ret) override;

  virtual void releaseTransponder(QByteArray& firmware,
                                  ReturnStatus& ret) override;
  virtual void confirmTransponderRelease(const StringDictionary& param,
                                         ReturnStatus& ret) override;
  virtual void rereleaseTransponder(const StringDictionary& param,
                                    QByteArray& firmware,
                                    ReturnStatus& ret) override;
  virtual void confirmTransponderRerelease(const StringDictionary& param,
                                           ReturnStatus& ret) override;
  virtual void rollbackTransponder(ReturnStatus& ret) override;
  virtual void getCurrentTransponderData(StringDictionary& data,
                                         ReturnStatus& ret) override;
  virtual void getTransponderData(const StringDictionary& param,
                                  StringDictionary& data,
                                  ReturnStatus& ret) override;

  virtual void printBoxStickerManually(const StringDictionary& param,
                                       ReturnStatus& ret) override;
  virtual void printLastBoxStickerManually(ReturnStatus& ret) override;
  virtual void printPalletStickerManually(const StringDictionary& param,
                                          ReturnStatus& ret) override;
  virtual void printLastPalletStickerManually(ReturnStatus& ret) override;

 private:
  ProductionDispatcher();
  Q_DISABLE_COPY_MOVE(ProductionDispatcher);

  void loadSettings(void);
  void sendLog(const QString& log);

  void initOperation(const QString& name);
  void processOperationError(const QString& name, ReturnStatus ret);
  void completeOperation(const QString& name);

  bool loadContext(QObject* obj);

  void createLaunchSystem(void);
  void createReleaseSystem(void);
  void createInfoSystem(void);
  void createStickerPrinters(void);
  void createDatabase(void);
  void createFirmwareGenerator(void);
  void createCheckTimer(void);

 private slots:
  void on_CheckTimerTemeout(void);

  void releaserBoxAssemblyComleted_slot(const std::shared_ptr<QString> id);
  void releaserPalletAssemblyComleted_slot(const std::shared_ptr<QString> id);
  void releaserOrderAssemblyComleted_slot(const std::shared_ptr<QString> id);
};

#endif  // PRODUCTIONDISPATCHER_H
