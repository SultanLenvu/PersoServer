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
  sendLog(QString("Запуск производственной линии. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Launcher->launch();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при запуске производственной линии '%1'. ")
                .arg(Context->login()));
    return;
  }

  sendLog(
      QString("Производственная линия '%1' запущена. ").arg(Context->login()));
}

void ProductionDispatcher::shutdownProductionLine(ReturnStatus& ret) {
  sendLog(QString("Остановка производственной линии. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Launcher->shutdown();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось остановить производственную линию '%1'. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Производственная линия '%1' остановлена. ")
              .arg(Context->login()));
  ret = ReturnStatus::NoError;
}

void ProductionDispatcher::requestBox(ReturnStatus& ret) {
  sendLog(QString("Запрос бокса. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Launcher->requestBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не получить бокс для производственной линиии '%1'. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Бокс успешно найден. "));
}

void ProductionDispatcher::getCurrentBoxData(StringDictionary& data,
                                             ReturnStatus& ret) {
  sendLog(QString("Получение данных бокса. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Informer->generateBoxData(data);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось найти данные текущего бокса на "
                    "производственной линиии %1. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Данные успешно получены. "));
}

void ProductionDispatcher::refundBox(ReturnStatus& ret) {
  sendLog(QString("Возврат бокса. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Launcher->refundBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось найти бокс для производственной линиии %1. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Бокс успешно возвращен. "));
}

void ProductionDispatcher::completeBox(ReturnStatus& ret) {
  sendLog(QString("Завершение сборки бокса. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Launcher->completeBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось найти бокс для производственной линиии %1. ")
                .arg(Context->login()));
    return;
  }

  StringDictionary boxData;
  ret = Informer->generateBoxData(boxData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  ret = BoxStickerPrinter->printBoxSticker(boxData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Сборка бокса успешно завершена. "));
}

void ProductionDispatcher::releaseTransponder(QByteArray& firmware,
                                              ReturnStatus& ret) {
  sendLog(QString("Выпуск транспондера. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Releaser->release();
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при выпуске транспондера.");
    emit errorDetected(ret);
    return;
  }

  StringDictionary seed;
  ret = Informer->generateFirmwareSeed(seed);
  if (ret != ReturnStatus::NoError) {
    sendLog(
        "Получена ошибка при генерации сида для генерации прошивки "
        "транспондера.");
    emit errorDetected(ret);
    return;
  }

  ret = Generator->generate(seed, firmware);
  if (ret != ReturnStatus::NoError) {
    sendLog(
        "Получена ошибка при генерации сида для генерации прошивки "
        "транспондера.");
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Транспондер успешно выпущен. "));
}

void ProductionDispatcher::confirmTransponderRelease(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Подтверждение выпуска транспондера. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Releaser->confirmRelease(param.value("transponder_ucid"));
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при выпуске транспондера.");
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Выпуск транспондера успешно подтвержден."));
}

void ProductionDispatcher::rereleaseTransponder(const StringDictionary& param,
                                                QByteArray& firmware,
                                                ReturnStatus& ret) {
  sendLog(QString("Перевыпуск транспондера. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  QString pan = param.value("transponder_pan")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  ret = Releaser->rerelease("personal_account_number", pan);
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при перевыпуске транспондера.");
    emit errorDetected(ret);
    return;
  }

  StringDictionary seed;
  ret = Informer->generateFirmwareSeed("personal_account_number", pan, seed);
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при генерации сида для прошивки транспондера.");
    emit errorDetected(ret);
    return;
  }

  ret = Generator->generate(seed, firmware);
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при генерации прошивки транспондера.");
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Транспондер успешно перевыпущен."));
}

void ProductionDispatcher::confirmTransponderRerelease(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Подтверждение перевыпуска транспондера. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  QString pan = param.value("transponder_pan")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  ret = Releaser->confirmRerelease("personal_account_number", pan,
                                   param.value("transponder_ucid"));
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при перевыпуске транспондера.");
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Перевыпуск транспондера успешно подтвержден."));
}

void ProductionDispatcher::rollbackTransponder(ReturnStatus& ret) {
  sendLog(QString("Откат транспондера. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Releaser->rollback();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные откатить транспондер.");
    return;
  }

  sendLog(QString("Откат транспондера успешно выполнен."));
}

void ProductionDispatcher::getCurrentTransponderData(StringDictionary& data,
                                                     ReturnStatus& ret) {
  sendLog(QString("Получение данных текущего транспондера. "));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Informer->generateTransponderData(data);
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные транспондера.");
    return;
  }

  sendLog(QString("Данные транспондера успешно получены."));
}

void ProductionDispatcher::getTransponderData(const StringDictionary& param,
                                              StringDictionary& data,
                                              ReturnStatus& ret) {
  sendLog(QString("Получение данных текущего транспондера"));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = Informer->generateTransponderData(
      "personal_account_number", param.value("personal_account_number"), data);
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные транспондера.");
    return;
  }

  sendLog(QString("Данные транспондера успешно получены."));
}

void ProductionDispatcher::printBoxStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Печать стикера для бокса."));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  QString boxId = Informer->getTransponderBoxId(
      "personal_account_number",
      param.value("pan").leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F')));

  StringDictionary boxData;
  ret = Informer->generateBoxData(boxId, boxData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  ret = BoxStickerPrinter->printBoxSticker(boxData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Стикер для бокса успешно распечатан."));
}

void ProductionDispatcher::printLastBoxStickerManually(ReturnStatus& ret) {
  sendLog(QString("Печать последнего стикера для бокса."));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = BoxStickerPrinter->printLastBoxSticker();

  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при повторной печати стикера для бокса."));
    return;
  }

  sendLog(QString("Последний стикер для бокса успешно распечатан."));
}

void ProductionDispatcher::printPalletStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Печать стикера для паллеты."));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  QString palletId = Informer->getTransponderPalletId(
      "personal_account_number",
      param.value("pan").leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F')));

  StringDictionary palletData;
  ret = Informer->generatePalletData(palletId, palletData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  ret = PalletStickerPrinter->printPalletSticker(palletData);
  if (ret != ReturnStatus::NoError) {
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Стикер для паллеты успешно распечатан."));
}

void ProductionDispatcher::printLastPalletStickerManually(ReturnStatus& ret) {
  sendLog(QString("Печать последнего стикера для паллеты"));
  if (!loadContext(sender())) {
    ret = ReturnStatus::ProductionLineContextNotAuthorized;
    return;
  }

  ret = PalletStickerPrinter->printLastPalletSticker();

  if (ret != ReturnStatus::NoError) {
    sendLog(
        QString("Получена ошибка при повторной печати стикера для паллеты."));
    return;
  }

  sendLog(QString("Последний стикер для паллеты успешно распечатан."));
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
