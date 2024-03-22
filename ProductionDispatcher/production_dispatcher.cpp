#include "production_dispatcher.h"
#include "box_release_system.h"
#include "config.h"
#include "firmware_generation_system.h"
#include "global_environment.h"
#include "info_system.h"
#include "postgre_sql_database.h"
#include "production_line_context_owner.h"
#include "production_line_launch_system.h"
#include "te310_printer.h"
#include "transponder_release_system.h"

ProductionDispatcher::ProductionDispatcher(const QString& name)
    : AbstractProductionDispatcher(name) {
  loadSettings();

  // Выбрасываем указатель на глобальный контекст
  GlobalEnvironment::instance()->registerObject(this);
}

ProductionDispatcher::~ProductionDispatcher() {}

void ProductionDispatcher::onInstanceThreadStarted() {
  MainContext = std::shared_ptr<ProductionContext>(new ProductionContext());

  createDatabase();

  createInfoSystem();
  createLaunchSystem();
  createBoxReleaseSystem();
  createReleaseSystem();

  createFirmwareGenerator();
  createStickerPrinters();

  createCriticalErrors();
}

void ProductionDispatcher::start(ReturnStatus& ret) {
  if (!Database->connect()) {
    sendLog("Не удалось подключиться к базе данных. ");
    ret = ReturnStatus::DatabaseConnectionError;
    return;
  }

  if (Launcher->init() != ReturnStatus::NoError) {
    sendLog("Инициализация системы запуска произвлдственных линий провалена. ");
    ret = ReturnStatus::ProductionLineLaunchSystemInitError;
    return;
  }

  if (!Generator->init()) {
    sendLog("Инициализация генератора прошивок провалена. ");
    ret = ReturnStatus::FirmwareGeneratorInitError;
    return;
  }

#ifdef PRINTERS_ENABLE
  ret = BoxStickerPrinter->checkConfig();
  if (ret != ReturnStatus::NoError) {
    return;
  }

  ret = PalletStickerPrinter->checkConfig();
  if (ret != ReturnStatus::NoError) {
    return;
  }
#endif

  ReturnStatus ret1 = Informer->updateMainContext();
  if (ret1 != ReturnStatus::NoError) {
    sendLog("Не удалось обновить производственный контекст.");
  }

  ret = ReturnStatus::NoError;
  sendLog("Запущен.");
}

void ProductionDispatcher::stop() {
  Database->disconnect();
  sendLog("Остановлен.");
}

void ProductionDispatcher::launchProductionLine(ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("launchProductionLine");

  ret = Launcher->launch();
  if (ret != ReturnStatus::NoError) {
    processOperationError("launchProductionLine", ret);
    return;
  }

  completeOperation("launchProductionLine");
}

void ProductionDispatcher::shutdownProductionLine(ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("shutdownProductionLine");

  ret = BoxReleaser->refund();
  if ((ret != ReturnStatus::NoError) &&
      (ret != ReturnStatus::ProductionLineNotInProcess)) {
    processOperationError("shutdownProductionLine", ret);
    return;
  }

  ret = Launcher->shutdown();
  if (ret != ReturnStatus::NoError) {
    processOperationError("shutdownProductionLine", ret);
    return;
  }

  completeOperation("shutdownProductionLine");
}

void ProductionDispatcher::getProductinoLineData(StringDictionary& data,
                                                 ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("getProductinoLineData");

  ret = Informer->generateProductionLineData(data);
  if (ret != ReturnStatus::NoError) {
    processOperationError("getProductinoLineData", ret);
    return;
  }

  completeOperation("getProductinoLineData");
}

void ProductionDispatcher::requestBox(ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }

  initOperation("requestBox");

  ret = BoxReleaser->request();
  if (ret != ReturnStatus::NoError) {
    processOperationError("requestBox", ret);
    return;
  }

  ret = TransponderReleaser->findLastReleased();
  if ((ret != ReturnStatus::NoError) &&
      (ret != ReturnStatus::TransponderMissed)) {
    processOperationError("requestBox", ret);
    return;
  }

  ret = ReturnStatus::NoError;
  completeOperation("requestBox");
}

void ProductionDispatcher::getCurrentBoxData(StringDictionary& data,
                                             ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("getCurrentBoxData");

  ret = Informer->generateBoxData(data);
  if (ret != ReturnStatus::NoError) {
    processOperationError("getCurrentBoxData", ret);
    return;
  }

  completeOperation("getCurrentBoxData");
}

void ProductionDispatcher::refundBox(ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("refundBox");

  ret = BoxReleaser->refund();
  if (ret != ReturnStatus::NoError) {
    processOperationError("refundBox", ret);
    return;
  }

  completeOperation("refundBox");
}

void ProductionDispatcher::completeBox(ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("completeBox");

  ret = BoxReleaser->complete();
  if (ret != ReturnStatus::NoError) {
    processOperationError("completeBox", ret);
    return;
  }

  completeOperation("completeBox");
}

void ProductionDispatcher::releaseTransponder(QByteArray& firmware,
                                              ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("releaseTransponder");

  ret = TransponderReleaser->findNext();
  if (ret != ReturnStatus::NoError) {
    processOperationError("releaseTransponder", ret);
    return;
  }

  ret = TransponderReleaser->release();
  if (ret != ReturnStatus::NoError) {
    processOperationError("releaseTransponder", ret);
    return;
  }

  StringDictionary seed;
  ret = Informer->generateFirmwareSeed(seed);
  if (ret != ReturnStatus::NoError) {
    processOperationError("releaseTransponder", ret);
    return;
  }

  ret = Generator->generate(seed, firmware);
  if (ret != ReturnStatus::NoError) {
    processOperationError("releaseTransponder", ret);
    return;
  }

  completeOperation("releaseTransponder");
}

void ProductionDispatcher::confirmTransponderRelease(
    const StringDictionary& param,
    ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("confirmTransponderRelease");

  ret = TransponderReleaser->confirmRelease(param.value("ucid"));
  if (ret != ReturnStatus::NoError) {
    processOperationError("confirmTransponderRelease", ret);
    return;
  }

  completeOperation("confirmTransponderRelease");
}

void ProductionDispatcher::rereleaseTransponder(const StringDictionary& param,
                                                QByteArray& firmware,
                                                ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("rereleaseTransponder");

  ret = TransponderReleaser->rerelease("personal_account_number",
                                       param.value("personal_account_number"));
  if (ret != ReturnStatus::NoError) {
    processOperationError("rereleaseTransponder", ret);
    return;
  }

  StringDictionary seed;
  ret = Informer->generateFirmwareSeed(
      "personal_account_number", param.value("personal_account_number"), seed);
  if (ret != ReturnStatus::NoError) {
    processOperationError("rereleaseTransponder", ret);
    return;
  }

  ret = Generator->generate(seed, firmware);
  if (ret != ReturnStatus::NoError) {
    processOperationError("rereleaseTransponder", ret);
    return;
  }

  completeOperation("rereleaseTransponder");
}

void ProductionDispatcher::confirmTransponderRerelease(
    const StringDictionary& param,
    ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("confirmTransponderRerelease");

  ret = TransponderReleaser->confirmRerelease(
      "personal_account_number", param.value("personal_account_number"),
      param.value("ucid"));
  if (ret != ReturnStatus::NoError) {
    processOperationError("confirmTransponderRelease", ret);
    return;
  }

  completeOperation("confirmTransponderRerelease");
}

void ProductionDispatcher::rollbackTransponder(ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("rollbackTransponder");

  ret = TransponderReleaser->rollback();
  if (ret != ReturnStatus::NoError) {
    processOperationError("rollbackTransponder", ret);
    return;
  }

  completeOperation("rollbackTransponder");
}

void ProductionDispatcher::getCurrentTransponderData(StringDictionary& data,
                                                     ReturnStatus& ret) {
  if (!MainContext->isValid()) {
    sendLog("Производственный контекст недоступен.");
    ret = ReturnStatus::ProductionContextNotValid;
    return;
  }

  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("getCurrentTransponderData");

  ret = Informer->generateTransponderData(data);
  if (ret != ReturnStatus::NoError) {
    processOperationError("getCurrentTransponderData", ret);
    return;
  }

  completeOperation("getCurrentTransponderData");
}

void ProductionDispatcher::getTransponderData(const StringDictionary& param,
                                              StringDictionary& data,
                                              ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("getTransponderData");

  ret = Informer->generateTransponderData(
      "personal_account_number", param.value("personal_account_number"), data);
  if (ret != ReturnStatus::NoError) {
    processOperationError("getTransponderData", ret);
    return;
  }

  completeOperation("getTransponderData");
}

void ProductionDispatcher::printBoxStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("printBoxStickerManually");

  QString boxId = Informer->getTransponderBoxId(
      "personal_account_number", param.value("personal_account_number"));

  StringDictionary boxData;
  ret = Informer->generateBoxData(boxId, boxData);
  if (ret != ReturnStatus::NoError) {
    processOperationError("printBoxStickerManually", ret);
    return;
  }

  ret = BoxStickerPrinter->printBoxSticker(boxData);
  if (ret != ReturnStatus::NoError) {
    processOperationError("printBoxStickerManually", ret);
    return;
  }

  completeOperation("printBoxStickerManually");
}

void ProductionDispatcher::printLastBoxStickerManually(ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("printLastBoxStickerManually");

  ret = BoxStickerPrinter->printLastBoxSticker();
  if (ret != ReturnStatus::NoError) {
    processOperationError("printLastBoxStickerManually", ret);
    return;
  }

  completeOperation("printLastBoxStickerManually");
}

void ProductionDispatcher::printPalletStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("printPalletStickerManually");

  QString palletId = Informer->getTransponderPalletId(
      "personal_account_number", param.value("personal_account_number"));

  StringDictionary palletData;
  ret = Informer->generatePalletData(palletId, palletData);
  if (ret != ReturnStatus::NoError) {
    processOperationError("printPalletStickerManually", ret);
    return;
  }

  ret = PalletStickerPrinter->printPalletSticker(palletData);
  if (ret != ReturnStatus::NoError) {
    processOperationError("printPalletStickerManually", ret);
    return;
  }

  completeOperation("printPalletStickerManually");
}

void ProductionDispatcher::printLastPalletStickerManually(ReturnStatus& ret) {
  ret = loadSubContext(sender());
  if (ret != ReturnStatus::NoError) {
    return;
  }
  initOperation("printLastPalletStickerManually");

  ret = PalletStickerPrinter->printLastPalletSticker();
  if (ret != ReturnStatus::NoError) {
    processOperationError("printLastPalletStickerManually", ret);
    return;
  }

  completeOperation("printLastPalletStickerManually");
}

void ProductionDispatcher::loadSettings() {
  QSettings settings;
}

void ProductionDispatcher::sendLog(const QString& log) {
  emit const_cast<ProductionDispatcher*>(this)->logging(objectName() + " - " +
                                                        log);
}

void ProductionDispatcher::initOperation(const QString& name) {
  sendLog(QString("Инициализация операции '%1'.").arg(name));
  SubContext->stash();
  MainContext->stash();
  Database->openTransaction();
}

void ProductionDispatcher::processOperationError(const QString& name,
                                                 ReturnStatus ret) {
  sendLog(QString("Получена ошибка при выполнении операции '%1'. ").arg(name));
  SubContext->applyStash();
  MainContext->applyStash();
  Database->rollbackTransaction();
  if (CriticalErrors.count(ret) > 0) {
    emit criticalErrorDetected(ret);
  }
}

void ProductionDispatcher::completeOperation(const QString& name) {
  sendLog(QString("Операция '%1' успешно выполнена. ").arg(name));
  Database->commitTransaction();
}

ReturnStatus ProductionDispatcher::loadSubContext(QObject* obj) {
  ProductionLineContextOwner* owner =
      static_cast<ProductionLineContextOwner*>(obj);

  SubContext = owner->context();
  if (!SubContext->isAuthorized()) {
    sendLog(QString(
        "Производственная линия не авторизирована. Контекст недоступен. "));
    return ReturnStatus::ProductionLineContextNotAuthorized;
  }

  TransponderReleaser->setSubContext(SubContext);
  BoxReleaser->setSubContext(SubContext);
  Informer->setSubContext(SubContext);
  Launcher->setSubContext(SubContext);

  sendLog(QString("Контекст производственной линии '%1' загружен.")
              .arg(SubContext->login()));

  ReturnStatus ret = ReturnStatus::NoError;
  if (!MainContext->isOrderReady()) {
    sendLog("Текущий заказ собран. Обновление контекста производства.");
    updateMainContext(ret);
    if (ret != ReturnStatus::NoError) {
      sendLog("Получена ошибка при обновлении контекста производства.");
      return ret;
    }
  }

  return ReturnStatus::NoError;
}

void ProductionDispatcher::createLaunchSystem() {
  Launcher = std::unique_ptr<AbstractLaunchSystem>(
      new ProductionLineLaunchSystem("ProductionLineLaunchSystem"));
  Launcher->setDatabase(Database);
  Launcher->setMainContext(MainContext);
}

void ProductionDispatcher::createBoxReleaseSystem() {
  BoxReleaser = std::unique_ptr<AbstractBoxReleaseSystem>(
      new BoxReleaseSystem("BoxReleaseSystem"));
  BoxReleaser->setDatabase(Database);
  BoxReleaser->setMainContext(MainContext);

  connect(BoxReleaser.get(), &AbstractBoxReleaseSystem::boxAssemblyCompleted,
          this, &ProductionDispatcher::processBoxAssemblyCompletion,
          Qt::DirectConnection);
  connect(BoxReleaser.get(), &AbstractBoxReleaseSystem::palletAssemblyCompleted,
          this, &ProductionDispatcher::processPalletAssemblyCompletion,
          Qt::DirectConnection);
  connect(BoxReleaser.get(), &AbstractBoxReleaseSystem::orderAssemblyCompleted,
          this, &ProductionDispatcher::processOrderAssemblyCompletion,
          Qt::DirectConnection);
}

void ProductionDispatcher::createReleaseSystem() {
  TransponderReleaser = std::unique_ptr<AbstractTransponderReleaseSystem>(
      new TransponderReleaseSystem("TransponderReleaseSystem"));
  TransponderReleaser->setDatabase(Database);
  TransponderReleaser->setMainContext(MainContext);
}

void ProductionDispatcher::createInfoSystem() {
  Informer = std::unique_ptr<AbstractInfoSystem>(new InfoSystem("InfoSystem"));
  Informer->setDatabase(Database);
  Informer->setMainContext(MainContext);
}

void ProductionDispatcher::createStickerPrinters() {
  BoxStickerPrinter = std::unique_ptr<AbstractStickerPrinter>(
      new TE310Printer("box_sticker_printer"));
  PalletStickerPrinter = std::unique_ptr<AbstractStickerPrinter>(
      new TE310Printer("pallet_sticker_printer"));
}

void ProductionDispatcher::createDatabase() {
  Database = std::shared_ptr<AbstractSqlDatabase>(
      new PostgreSqlDatabase("PostgreSqlDatabase"));
}

void ProductionDispatcher::createFirmwareGenerator() {
  Generator = std::unique_ptr<AbstractFirmwareGenerationSystem>(
      new FirmwareGenerationSystem("FirmwareGenerationSystem"));
}

void ProductionDispatcher::createCriticalErrors() {
  CriticalErrors.insert(ReturnStatus::DynamicLibraryMissing);
  CriticalErrors.insert(ReturnStatus::ParameterError);
  CriticalErrors.insert(ReturnStatus::SyntaxError);
  CriticalErrors.insert(ReturnStatus::ConsistencyViolation);
  CriticalErrors.insert(ReturnStatus::FileOpenError);

  CriticalErrors.insert(ReturnStatus::DatabaseConnectionError);
  CriticalErrors.insert(ReturnStatus::DatabaseTransactionError);
  CriticalErrors.insert(ReturnStatus::DatabaseQueryError);

  CriticalErrors.insert(ReturnStatus::FirmwareGeneratorInitError);
  CriticalErrors.insert(ReturnStatus::FirmwareGenerationError);
}

void ProductionDispatcher::updateMainContext(ReturnStatus& ret) {
  ret = Informer->updateMainContext();
}

void ProductionDispatcher::processBoxAssemblyCompletion(ReturnStatus& ret) {
  QString id = SubContext->box().get("id");
  sendLog(QString("Обработка завершения сборки бокса %1.").arg(id));

  StringDictionary boxData;

  ret = Informer->generateBoxData(boxData);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось сгенерировать данные для бокса %1.").arg(id));
    return;
  }

  ret = BoxStickerPrinter->printBoxSticker(boxData);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось распечатать стикер для бокса %1.").arg(id));
    return;
  }

  ret = ReturnStatus::NoError;
}

void ProductionDispatcher::processPalletAssemblyCompletion(ReturnStatus& ret) {
  QString id = SubContext->box().get("pallet_id");
  sendLog(QString("Обработка завершения сборки паллеты %1.")
              .arg(SubContext->box().get("pallet_id")));

  StringDictionary palletData;

  ret = Informer->generatePalletData(palletData);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось сгенерировать данные для паллеты %1.").arg(id));
    return;
  }

  ret = PalletStickerPrinter->printPalletSticker(palletData);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось распечатать стикер для паллеты %1.").arg(id));
    return;
  }

  MainContext->removePallet(id);

  ret = ReturnStatus::NoError;
}

void ProductionDispatcher::processOrderAssemblyCompletion(ReturnStatus& ret) {
  sendLog(QString("Обработка завершения сборки заказа %1.")
              .arg(MainContext->order().get("id")));

  ret = ReturnStatus::NoError;
}
