#include "general_production_dispatcher.h"
#include "Database/postgre_sql_database.h"
#include "Log/log_system.h"
#include "ProductionDispatcher/te310_printer.h"
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
    StringDictionary& context,
    ReturnStatus& ret) {
  sendLog(QString("Запуск производственной линии '%1'. ")
              .arg(param.value("login")));

  ret = Launcher->launch(param);
  if (ret != ReturnStatus::NoError) {
    return;
  }

  // Если запуск успешный, то подгружаем контекст производственной линии
  std::unique_ptr<ProductionContext> newContext(new ProductionContext());
  Informer->generateProductionContext(param.value("login"), *newContext);
  Contexts.insert(param.value("login"), newContext);

  sendLog(QString("Контекст производственной линии '%1' загружен. ")
              .arg(param.value("login")));
  ret = ReturnStatus::NoError;
}

void GeneralProductionDispatcher::shutdownProductionLine(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog(QString("Остановка производственной линии '%1'. ")
              .arg(param.value("login")));

  ret = Launcher->shutdown(param);
  if (ret != ReturnStatus::NoError) {
    return;
  }

  // Если остановка успешная, то выгружаем контекст производственной линии
  Contexts.remove(param.value("login"));

  sendLog(QString("Контекст производственной линии '%1' загружен. ")
              .arg(param.value("login")));
  ret = ReturnStatus::NoError;
}

void GeneralProductionDispatcher::getProductionLineContext(
    const StringDictionary& param,
    StringDictionary& result,
    ReturnStatus& ret) {
  sendLog(QString("Запрос контекста производственной линии '%1'. ")
              .arg(param.value("login")));

  if (!Contexts.contains(param.value("login"))) {
    sendLog(
        QString(
            "Производственная линия '%1' не запущена, контекст недоступен. ")
            .arg(param.value("login")));

    ret = ReturnStatus::UnauthorizedRequest;
    return;
  }
}

void GeneralProductionDispatcher::rollbackProductionLine(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus& ret) {}

void GeneralProductionDispatcher::releaseTransponder(
    const StringDictionary& param,
    StringDictionary& seed,
    ReturnStatus& ret) {}

void GeneralProductionDispatcher::confirmTransponderRelease(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus& ret) {}

void GeneralProductionDispatcher::rereleaseTransponder(
    const StringDictionary& param,
    StringDictionary& seed,
    ReturnStatus& ret) {}

void GeneralProductionDispatcher::confirmTransponderRerelease(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus& ret) {}

void GeneralProductionDispatcher::printBoxStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog("Запуск печати стикера для бокса.");

  // Не забудь дополнить PAN
  //  .toString().leftJustified(
  //                          FULL_PAN_CHAR_LENGTH, QChar('F'))

  AbstractStickerPrinter::ReturnStatus status;
  status = BoxStickerPrinter->printBoxSticker(param);
}

void GeneralProductionDispatcher::printLastBoxStickerManually(
    ReturnStatus& ret) {
  sendLog("Запуск печати последнего стикера для бокса.");

  AbstractStickerPrinter::ReturnStatus status;
  status = BoxStickerPrinter->printLastBoxSticker();
}

void GeneralProductionDispatcher::printPalletStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  sendLog("Запуск печати стикера для паллеты.");

  AbstractStickerPrinter::ReturnStatus status;
  status = PalletStickerPrinter->printPalletSticker(param);
}

void GeneralProductionDispatcher::printLastPalletStickerManually(
    ReturnStatus& ret) {
  sendLog("Запуск печати последнего стикера для паллеты.");

  AbstractStickerPrinter::ReturnStatus status;
  status = PalletStickerPrinter->printLastPalletSticker();
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

void GeneralProductionDispatcher::createLaunchSystem() {
  Launcher = std::unique_ptr<AbstractLaunchSystem>(
      new ProductionLineLaunchSystem("TransponderReleaseSystem", Database));
}

void GeneralProductionDispatcher::createReleaseSystem() {
  Releaser = std::unique_ptr<AbstractReleaseSystem>(
      new TransponderReleaseSystem("TransponderReleaseSystem", Database));
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
