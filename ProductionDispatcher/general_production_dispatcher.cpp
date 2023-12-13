#include "general_production_dispatcher.h"
#include "Database/postgre_sql_database.h"
#include "Log/log_system.h"
#include "Printing/te310_printer.h"
#include "firmware_generation_system.h"
#include "info_system.h"
#include "production_line_launch_system.h"
#include "transponder_release_system.h"

GeneralProductionDispatcher::GeneralProductionDispatcher(const QString& name)
    : AbstractProductionDispatcher(name) {
  loadSettings();

  createDatabase();
  createInfoSystem();
  createLaunchSystem();
  createReleaseSystem();
  createFirmwareGenerator();
  createStickerPrinters();
}

GeneralProductionDispatcher::~GeneralProductionDispatcher() {}

void GeneralProductionDispatcher::start(ReturnStatus& ret) {
  if (!Generator->init()) {
    sendLog("Инициализация генератора прошивок провалена. ");
    ret = ReturnStatus::FirmwareGeneratorInitError;
    return;
  }

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

  if (!Database->connect()) {
    sendLog("Не удалось подключиться к базе данных. ");
    ret = ReturnStatus::DatabaseConnectionError;
    return;
  }

  CheckTimer->start();
  ret = ReturnStatus::NoError;
  sendLog("Запущен.");
}

void GeneralProductionDispatcher::stop() {
  Database->disconnect();
  sendLog("Остановлен.");
}

void GeneralProductionDispatcher::launchProductionLine(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Запуск производственной линии '%1'. ")
              .arg(param.value("login")));

  ret = Launcher->launch(param);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при запуске производственной линии '%1'. ")
                .arg(param.value("login")));
    return;
  }

  // Если запуск успешный, то подгружаем контекст производственной линии
  std::shared_ptr<ProductionLineContext> newContext(
      new ProductionLineContext());
  Informer->loadProductionLineContext(param.value("login"), *newContext);
  Contexts.insert(param.value("login"), newContext);

  sendLog(QString("Контекст производственной линии '%1' загружен. ")
              .arg(param.value("login")));
}

void GeneralProductionDispatcher::shutdownProductionLine(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Остановка производственной линии '%1'. ")
              .arg(param.value("login")));

  ret = Launcher->shutdown(param);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось остановить производственную линию '%1'. ")
                .arg(param.value("login")));
    return;
  }

  // Выгружаем контекст производственной линии
  Contexts.remove(param.value("login"));

  sendLog(QString("Контекст производственной линии '%1' удален. ")
              .arg(param.value("login")));
}

void GeneralProductionDispatcher::getProductionLineContext(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus& ret) {
  sendLog(QString("Запрос контекста производственной линии '%1'. ")
              .arg(param.value("login")));

  if (!Contexts.contains(param.value("login"))) {
    sendLog(
        QString(
            "Производственная линия '%1' не запущена. Контекст недоступен. ")
            .arg(param.value("login")));
    ret = ReturnStatus::UnauthorizedRequest;
    return;
  }

  ret = Informer->generateTransponderData(context);
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные транспондера.");
    return;
  }
}

void GeneralProductionDispatcher::rollbackProductionLine(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(
      QString("Откат производственной линии '%1'. ").arg(param.value("login")));

  if (!Contexts.contains(param.value("login"))) {
    sendLog(
        QString("Производственная линия '%1' не запущена. Отка невозможен. ")
            .arg(param.value("login")));
    ret = ReturnStatus::UnauthorizedRequest;
    return;
  }

  switchCurrentContext(param.value("login"));
  ret = Releaser->rollback();
}

void GeneralProductionDispatcher::releaseTransponder(
    const StringDictionary& param,
    QByteArray& firmware,
    ReturnStatus& ret) {
  sendLog(QString("Выпуск транспондера на производственной линии '%1'. ")
              .arg(param.value("login")));

  if (!Contexts.contains(param.value("login"))) {
    sendLog(QString("Производственная линия '%1' не запущена. Выпуск "
                    "транспондера невозможен. ")
                .arg(param.value("login")));
    ret = ReturnStatus::UnauthorizedRequest;
    return;
  }

  switchCurrentContext(param.value("login"));

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

void GeneralProductionDispatcher::confirmTransponderRelease(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(
      QString(
          "Подтверждение выпуска транспондера на производственной линии '%1'. ")
          .arg(param.value("login")));

  if (!Contexts.contains(param.value("login"))) {
    sendLog(
        QString(
            "Производственная линия '%1' не запущена. Подтверждение выпуска "
            "транспондера невозможно. ")
            .arg(param.value("login")));
    ret = ReturnStatus::UnauthorizedRequest;
    return;
  }

  switchCurrentContext(param.value("login"));

  ret = Releaser->confirmRelease(param.value("transponder_ucid"));
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при выпуске транспондера.");
    emit errorDetected(ret);
    return;
  }

  sendLog(QString("Выпуск транспондера успешно подтвержден."));
}

void GeneralProductionDispatcher::rereleaseTransponder(
    const StringDictionary& param,
    QByteArray& firmware,
    ReturnStatus& ret) {
  sendLog(QString("Перевыпуск транспондера. "));

  QString pan = param.value("transponder_pan")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  ret = Releaser->rerelease("personal_account_number", pan);
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при выпуске транспондера.");
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

void GeneralProductionDispatcher::confirmTransponderRerelease(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Подтверждение перевыпуска транспондера. "));

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

void GeneralProductionDispatcher::printBoxStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog("Запуск печати стикера для бокса.");

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
}

void GeneralProductionDispatcher::printLastBoxStickerManually(
    ReturnStatus& ret) {
  sendLog("Запуск печати последнего стикера для бокса.");

  ret = BoxStickerPrinter->printLastBoxSticker();
}

void GeneralProductionDispatcher::printPalletStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog("Запуск печати стикера для паллеты.");

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
}

void GeneralProductionDispatcher::printLastPalletStickerManually(
    ReturnStatus& ret) {
  sendLog("Запуск печати последнего стикера для паллеты.");

  ret = PalletStickerPrinter->printLastPalletSticker();
}

void GeneralProductionDispatcher::loadSettings() {
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

void GeneralProductionDispatcher::sendLog(const QString& log) {
  LogSystem::instance()->generate(objectName() + " - " + log);
}

void GeneralProductionDispatcher::switchCurrentContext(const QString& name) {
  Releaser->setContext(*Contexts.value(name));
  Informer->setContext(*Contexts.value(name));
}

void GeneralProductionDispatcher::createLaunchSystem() {
  Launcher = std::unique_ptr<AbstractLaunchSystem>(
      new ProductionLineLaunchSystem("TransponderReleaseSystem", Database));
}

void GeneralProductionDispatcher::createReleaseSystem() {
  Releaser = std::unique_ptr<AbstractReleaseSystem>(
      new TransponderReleaseSystem("TransponderReleaseSystem", Database));

  connect(Releaser.get(), &AbstractReleaseSystem::boxAssemblyCompleted, this,
          &GeneralProductionDispatcher::releaserBoxAssemblyComleted_slot);
  connect(Releaser.get(), &AbstractReleaseSystem::palletAssemblyCompleted, this,
          &GeneralProductionDispatcher::releaserPalletAssemblyComleted_slot);
  connect(Releaser.get(), &AbstractReleaseSystem::orderAssemblyCompleted, this,
          &GeneralProductionDispatcher::releaserOrderAssemblyComleted_slot);
}

void GeneralProductionDispatcher::createInfoSystem() {
  Informer = std::unique_ptr<AbstractInfoSystem>(
      new InfoSystem("InfoSystem", Database));
}

void GeneralProductionDispatcher::createStickerPrinters() {
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

void GeneralProductionDispatcher::createDatabase() {
  Database = std::shared_ptr<AbstractSqlDatabase>(
      new PostgreSqlDatabase("PostgreSqlDatabase"));
}

void GeneralProductionDispatcher::createCheckTimer() {
  CheckTimer = std::unique_ptr<QTimer>(new QTimer());
  CheckTimer->setInterval(CheckPeriod * 1000);

  connect(CheckTimer.get(), &QTimer::timeout, CheckTimer.get(), &QTimer::stop);
  connect(CheckTimer.get(), &QTimer::timeout, this,
          &GeneralProductionDispatcher::on_CheckTimerTemeout);
}

void GeneralProductionDispatcher::createFirmwareGenerator() {
  Generator = std::unique_ptr<AbstractFirmwareGenerationSystem>(
      new FirmwareGenerationSystem("FirmwareGenerationSystem"));
}

void GeneralProductionDispatcher::on_CheckTimerTemeout() {
  if (!Database->isConnected()) {
    sendLog("Потеряно соединение с базой данных.");
    emit errorDetected(ReturnStatus::DatabaseConnectionError);
  }
}

void GeneralProductionDispatcher::releaserBoxAssemblyComleted_slot(
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

void GeneralProductionDispatcher::releaserPalletAssemblyComleted_slot(
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

void GeneralProductionDispatcher::releaserOrderAssemblyComleted_slot(
    const std::shared_ptr<QString> id) {}
