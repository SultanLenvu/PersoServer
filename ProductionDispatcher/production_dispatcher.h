#ifndef PRODUCTIONDISPATCHER_H
#define PRODUCTIONDISPATCHER_H

#include <unordered_set>

#include "abstract_box_release_system.h"
#include "abstract_firmware_generation_system.h"
#include "abstract_info_system.h"
#include "abstract_launch_system.h"
#include "abstract_production_dispatcher.h"
#include "abstract_sql_database.h"
#include "abstract_sticker_printer.h"
#include "abstract_transponder_release_system.h"
#include "production_context.h"

class ProductionDispatcher : public AbstractProductionDispatcher {
  Q_OBJECT
 private:
  std::shared_ptr<AbstractSqlDatabase> Database;
  std::shared_ptr<ProductionContext> MainContext;
  std::shared_ptr<ProductionLineContext> SubContext;

  std::unique_ptr<AbstractInfoSystem> Informer;
  std::unique_ptr<AbstractLaunchSystem> Launcher;
  std::unique_ptr<AbstractBoxReleaseSystem> BoxReleaser;
  std::unique_ptr<AbstractTransponderReleaseSystem> TransponderReleaser;
  std::unique_ptr<AbstractFirmwareGenerationSystem> Generator;

  std::unique_ptr<AbstractStickerPrinter> BoxStickerPrinter;
  std::unique_ptr<AbstractStickerPrinter> PalletStickerPrinter;

  std::unordered_set<ReturnStatus> CriticalErrors;

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
  virtual void getProductinoLineData(StringDictionary& data,
                                     ReturnStatus& ret) override;

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

  ReturnStatus loadContext(QObject* obj);

  void createLaunchSystem(void);
  void createBoxReleaseSystem(void);
  void createReleaseSystem(void);
  void createInfoSystem(void);
  void createStickerPrinters(void);
  void createDatabase(void);
  void createFirmwareGenerator(void);
  void createCriticalErrors(void);

  void updateMainContext(ReturnStatus& ret);

 private slots:
  void processBoxAssemblyCompletion(void);
  void processPalletAssemblyCompletion(void);
  void processOrderAssemblyCompletion(void);
};

#endif  // PRODUCTIONDISPATCHER_H
