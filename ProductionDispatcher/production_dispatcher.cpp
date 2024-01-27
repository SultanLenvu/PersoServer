#include "production_dispatcher.h"
#include "definitions.h"
#include "firmware_generation_system.h"
#include "global_environment.h"
#include "info_system.h"
#include "postgre_sql_database.h"
#include "production_context_owner.h"
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
  createDatabase();
  createInfoSystem();
  createLaunchSystem();
  createReleaseSystem();
  createFirmwareGenerator();
  createStickerPrinters();
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
  if (!BoxStickerPrinter->init()) {
    sendLog(
        "Инициализация принтера для печати стикеров на боксы "
        "провалена. ");
    ret = ReturnStatus::StickerPrinterInitError;
    return;
  }

  if (!PalletStickerPrinter->init()) {
    sendLog(
        "Инициализация принтера для печати стикеров на паллеты "
        "провалена. ");
    ret = ReturnStatus::StickerPrinterInitError;
    return;
  }
#endif

  createCheckTimer();
  CheckTimer->start();
  ret = ReturnStatus::NoError;
  sendLog("Запущен.");
}

void ProductionDispatcher::stop() {
  Database->disconnect();
  sendLog("Остановлен.");
}

void ProductionDispatcher::launchProductionLine(ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("shutdownProductionLine");

  ret = Launcher->shutdown();
  if (ret != ReturnStatus::NoError) {
    processOperationError("shutdownProductionLine", ret);
    return;
  }

  completeOperation("shutdownProductionLine");
}

void ProductionDispatcher::requestBox(ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("requestBox");

  ret = Launcher->requestBox();
  if (ret != ReturnStatus::NoError) {
    processOperationError("requestBox", ret);
    return;
  }

  ret = Releaser->findLastReleased();
  if (ret != ReturnStatus::NoError) {
    processOperationError("requestBox", ret);
    return;
  }

  completeOperation("requestBox");
}

void ProductionDispatcher::getCurrentBoxData(StringDictionary& data,
                                             ReturnStatus& ret) {
  initOperation("getCurrentBoxData");
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Informer->generateBoxData(data);
  if (ret != ReturnStatus::NoError) {
    processOperationError("getCurrentBoxData", ret);
    return;
  }

  completeOperation("getCurrentBoxData");
}

void ProductionDispatcher::refundBox(ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("refundBox");

  ret = Launcher->refundBox();
  if (ret != ReturnStatus::NoError) {
    processOperationError("refundBox", ret);
    return;
  }

  // Очищаем соответствующий контекст
  Context->box().clear();
  Context->pallet().clear();
  Context->transponder().clear();

  completeOperation("refundBox");
}

void ProductionDispatcher::completeBox(ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("completeBox");

  ret = Launcher->completeBox();
  if (ret != ReturnStatus::NoError) {
    processOperationError("completeBox", ret);
    return;
  }

  StringDictionary boxData;
  ret = Informer->generateBoxData(boxData);
  if (ret != ReturnStatus::NoError) {
    processOperationError("completeBox", ret);
    return;
  }

  ret = BoxStickerPrinter->printBoxSticker(boxData);
  if (ret != ReturnStatus::NoError) {
    processOperationError("completeBox", ret);
    return;
  }

  completeOperation("completeBox");
}

void ProductionDispatcher::releaseTransponder(QByteArray& firmware,
                                              ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("releaseTransponder");

  ret = Releaser->findNext();
  if (ret != ReturnStatus::NoError) {
    processOperationError("confirmTransponderRelease", ret);
    return;
  }

  ret = Releaser->release();
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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("confirmTransponderRelease");

  ret = Releaser->confirmRelease(param.value("transponder_ucid"));
  if (ret != ReturnStatus::NoError) {
    processOperationError("confirmTransponderRelease", ret);
    return;
  }

  completeOperation("confirmTransponderRelease");
}

void ProductionDispatcher::rereleaseTransponder(const StringDictionary& param,
                                                QByteArray& firmware,
                                                ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("rereleaseTransponder");

  QString pan = param.value("transponder_pan")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  ret = Releaser->rerelease("personal_account_number", pan);
  if (ret != ReturnStatus::NoError) {
    processOperationError("rereleaseTransponder", ret);
    return;
  }

  StringDictionary seed;
  ret = Informer->generateFirmwareSeed("personal_account_number", pan, seed);
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
  initOperation("confirmTransponderRerelease");
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  QString pan = param.value("transponder_pan")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  ret = Releaser->confirmRerelease("personal_account_number", pan,
                                   param.value("transponder_ucid"));
  if (ret != ReturnStatus::NoError) {
    processOperationError("confirmTransponderRerelease", ret);
    return;
  }

  completeOperation("confirmTransponderRerelease");
}

void ProductionDispatcher::rollbackTransponder(ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("rollbackTransponder");

  ret = Releaser->rollback();
  if (ret != ReturnStatus::NoError) {
    processOperationError("rollbackTransponder", ret);
    return;
  }

  completeOperation("rollbackTransponder");
}

void ProductionDispatcher::getCurrentTransponderData(StringDictionary& data,
                                                     ReturnStatus& ret) {
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("printBoxStickerManually");

  QString boxId = Informer->getTransponderBoxId(
      "personal_account_number",
      param.value("pan").leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F')));

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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }
  initOperation("printPalletStickerManually");

  QString palletId = Informer->getTransponderPalletId(
      "personal_account_number",
      param.value("pan").leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F')));

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
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
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

#ifdef __linux__
  if (settings.contains("perso_server/box_sticker_printer_ip")) {
    /* Prescense of box_sticker_printer_port is checked during validation */
    BoxStickerPrinterIP = QHostAddress(
        settings.value("perso_server/box_sticker_printer_ip").toString());
    BoxStickerPrinterPort =
        settings.value("perso_server/box_sticker_printer_port").toInt();
  }

  if (settings.contains("perso_server/pallet_sticker_printer_ip")) {
    /*
     * Prescense of printer_for_pallet_sticker_port is checked during validation
     */
    PalletStickerPrinterIP = QHostAddress(
        settings.value("perso_server/pallet_sticker_printer_ip").toString());
    PalletStickerPrinterPort =
        settings.value("perso_server/pallet_sticker_printer_port").toInt();
  }
#else
  BoxStickerPrinterName =
      settings.value("perso_server/box_sticker_printer_name").toString();
  PalletStickerPrinterName =
      settings.value("perso_server/pallet_sticker_printer_name").toString();
#endif /* __linux__ */
}

void ProductionDispatcher::sendLog(const QString& log) {
  emit const_cast<ProductionDispatcher*>(this)->logging(objectName() + " - " +
                                                        log);
}

void ProductionDispatcher::initOperation(const QString& name) {
  sendLog(QString("Инициализация операции '%1'").arg(name));
  Context->stash();
  Database->openTransaction();
}

void ProductionDispatcher::processOperationError(const QString& name,
                                                 ReturnStatus ret) {
  sendLog(QString("Получена ошибка при выполнении операции '%1'. ").arg(name));
  Context->applyStash();
  Database->rollbackTransaction();
  emit errorDetected(ret);
}

void ProductionDispatcher::completeOperation(const QString& name) {
  sendLog(QString("Операция '%1' успешно выполнена. ").arg(name));
  Database->commitTransaction();
}

bool ProductionDispatcher::loadContext(QObject* obj) {
  ProductionContextOwner* owner = dynamic_cast<ProductionContextOwner*>(obj);
  assert(owner);

  Context = owner->context();
  if (!Context->isAuthorized()) {
    sendLog(QString(
        "Производственная линия не авторизирована. Контекст недоступен. "));
    return false;
  }

  Releaser->setContext(Context);
  Informer->setContext(Context);
  Launcher->setContext(Context);

  sendLog(QString("Контекст производственной линии '%1' загружен.")
              .arg(Context->login()));
  return true;
}

void ProductionDispatcher::createLaunchSystem() {
  Launcher = std::unique_ptr<AbstractLaunchSystem>(
      new ProductionLineLaunchSystem("ProductionLineLaunchSystem", Database));
}

void ProductionDispatcher::createReleaseSystem() {
  Releaser = std::unique_ptr<AbstractReleaseSystem>(
      new TransponderReleaseSystem("TransponderReleaseSystem", Database));
}

void ProductionDispatcher::createInfoSystem() {
  Informer = std::unique_ptr<AbstractInfoSystem>(
      new InfoSystem("InfoSystem", Database));
}

void ProductionDispatcher::createStickerPrinters() {
#ifdef __linux__
  BoxStickerPrinter =
      new TE310Printer(this, BoxStickerPrinterIP, BoxStickerPrinterPort);
#else
  BoxStickerPrinter = std::unique_ptr<AbstractStickerPrinter>(
      new TE310Printer(BoxStickerPrinterName));
#endif /* __linux__ */

#ifdef __linux__
  PalletStickerPrinter =
      new TE310Printer(this, PalletStickerPrinterIP, PalletStickerPrinterPort);
#else
  PalletStickerPrinter = std::unique_ptr<AbstractStickerPrinter>(
      new TE310Printer(PalletStickerPrinterName));
#endif /* __linux__ */
}

void ProductionDispatcher::createDatabase() {
  Database = std::shared_ptr<AbstractSqlDatabase>(
      new PostgreSqlDatabase("PostgreSqlDatabase"));
}

void ProductionDispatcher::createCheckTimer() {
  CheckTimer = std::unique_ptr<QTimer>(new QTimer());
  CheckTimer->setInterval(1000);

  connect(CheckTimer.get(), &QTimer::timeout, CheckTimer.get(), &QTimer::stop);
  connect(CheckTimer.get(), &QTimer::timeout, this,
          &ProductionDispatcher::on_CheckTimerTemeout);
}

void ProductionDispatcher::createFirmwareGenerator() {
  Generator = std::unique_ptr<AbstractFirmwareGenerationSystem>(
      new FirmwareGenerationSystem("FirmwareGenerationSystem"));
}

void ProductionDispatcher::on_CheckTimerTemeout() {
  if (!Database->isConnected()) {
    sendLog("Потеряно соединение с базой данных.");
    emit errorDetected(ReturnStatus::DatabaseConnectionError);
  }
}

void ProductionDispatcher::releaserBoxAssemblyComleted_slot(
    const std::shared_ptr<QString> id) {
  StringDictionary boxData;
  ReturnStatus ret;

  sendLog("Обработка сигнала для печати стикера для бокса.");

  ret = Informer->generateBoxData(*id, boxData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  ret = BoxStickerPrinter->printBoxSticker(boxData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }
}

void ProductionDispatcher::releaserPalletAssemblyComleted_slot(
    const std::shared_ptr<QString> id) {
  StringDictionary palletData;
  ReturnStatus ret;

  sendLog("Обработка сигнала для печати стикера для паллеты.");

  ret = Informer->generatePalletData(*id, palletData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  ret = PalletStickerPrinter->printPalletSticker(palletData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }
}

void ProductionDispatcher::releaserOrderAssemblyComleted_slot(
    const std::shared_ptr<QString> id) {}
