#include "general_production_dispatcher.h"
#include "Database/postgre_sql_database.h"
#include "Printing/te310_printer.h"
#include "abstract_client_command.h"
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

  if (!Database->connect()) {
    sendLog("Не удалось подключиться к базе данных. ");
    ret = ReturnStatus::DatabaseConnectionError;
    return;
  }

  createCheckTimer();
  CheckTimer->start();
  ret = ReturnStatus::NoError;
  sendLog("Запущен.");
}

void GeneralProductionDispatcher::stop() {
  Database->disconnect();
  sendLog("Остановлен.");
}

void GeneralProductionDispatcher::launchProductionLine(ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(
      QString("Запуск производственной линии '%1'. ").arg(Context->login()));

  ret = Launcher->launch();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при запуске производственной линии '%1'. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Производственная линия '%1' успешно запущена. ")
              .arg(Context->login()));
}

void GeneralProductionDispatcher::shutdownProductionLine(ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(
      QString("Остановка производственной линии '%1'. ").arg(Context->login()));

  ret = Launcher->shutdown();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось остановить производственную линию '%1'. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Производственная линия '%1' остановлена. ")
              .arg(Context->login()));
}

void GeneralProductionDispatcher::requestBox(ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос бокса на производственной линии '%1'. ")
              .arg(Context->login()));

  ret = Launcher->findBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось найти бокс для производственной линиии %1. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Бокс успешно найден. "));
}

void GeneralProductionDispatcher::getCurrentBoxData(StringDictionary& data,
                                                    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос данных бокса на производственной линии '%1'. ")
              .arg(Context->login()));

  ret = Informer->generateBoxData(data);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось найти данные текущего бокса на "
                    "производственной линиии %1. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Данные успешно получены. "));
}

void GeneralProductionDispatcher::refundBox(ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Возврат бокса на производственной линии '%1'. ")
              .arg(Context->login()));

  ret = Launcher->refundBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось найти бокс для производственной линиии %1. ")
                .arg(Context->login()));
    return;
  }

  sendLog(QString("Бокс успешно возвращен. "));
}

void GeneralProductionDispatcher::completeBox(ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Завершение сборки бокса на производственной линии '%1'. ")
              .arg(Context->login()));

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

void GeneralProductionDispatcher::releaseTransponder(QByteArray& firmware,
                                                     ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Выпуск транспондера на производственной линии '%1'. ")
              .arg(Context->login()));

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
  loadCurrentContext(sender());
  sendLog(
      QString(
          "Подтверждение выпуска транспондера на производственной линии '%1'. ")
          .arg(Context->login()));

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
  sendLog(QString("Перевыпуск транспондера на производственной линии %1. ")
              .arg(Context->login()));

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
  loadCurrentContext(sender());
  sendLog(QString("Подтверждение перевыпуска транспондера на производственной "
                  "линии %1. ")
              .arg(Context->login()));

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

void GeneralProductionDispatcher::rollbackTransponder(ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Откат транспондера на производственной линии '%1'. ")
              .arg(Context->login()));

  ret = Releaser->rollback();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные откатить транспондер.");
    return;
  }

  sendLog(QString("Откат транспондера успешно выполнен."));
}

void GeneralProductionDispatcher::getCurrentTransponderData(
    StringDictionary& data,
    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос данных текущего транспондера на производственной "
                  "линии '%1'. ")
              .arg(Context->login()));

  ret = Informer->generateTransponderData(data);
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные транспондера.");
    return;
  }

  sendLog(QString("Данные транспондера успешно получены."));
}

void GeneralProductionDispatcher::getTransponderData(
    const StringDictionary& param,
    StringDictionary& data,
    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос данных текущего транспондера на производственной "
                  "линии '%1'. ")
              .arg(Context->login()));

  ret = Informer->generateTransponderData(
      "personal_account_number", param.value("personal_account_number"), data);
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось получить данные транспондера.");
    return;
  }

  sendLog(QString("Данные транспондера успешно получены."));
}

void GeneralProductionDispatcher::printBoxStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(
      QString("Запрос печати стикера для бокса на производственной линии %1.")
          .arg(Context->login()));

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

void GeneralProductionDispatcher::printLastBoxStickerManually(
    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос печати последнего стикера для бокса на "
                  "производственной линии %1.")
              .arg(Context->login()));

  ret = BoxStickerPrinter->printLastBoxSticker();

  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при повторной печати стикера для бокса."));
    return;
  }

  sendLog(QString("Последний стикер для бокса успешно распечатан."));
}

void GeneralProductionDispatcher::printPalletStickerManually(
    const StringDictionary& param,
    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос печати стикера для паллеты на "
                  "производственной линии %1.")
              .arg(Context->login()));

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

void GeneralProductionDispatcher::printLastPalletStickerManually(
    ReturnStatus& ret) {
  loadCurrentContext(sender());
  sendLog(QString("Запрос печати последнего стикера для паллеты на "
                  "производственной линии %1.")
              .arg(Context->login()));

  ret = PalletStickerPrinter->printLastPalletSticker();

  if (ret != ReturnStatus::NoError) {
    sendLog(
        QString("Получена ошибка при повторной печати стикера для паллеты."));
    return;
  }

  sendLog(QString("Последний стикер для паллеты успешно распечатан."));
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
  emit const_cast<GeneralProductionDispatcher*>(this)->logging(objectName() +
                                                               " - " + log);
}

void GeneralProductionDispatcher::loadCurrentContext(QObject* obj) {
  AbstractClientCommand* cmd = dynamic_cast<AbstractClientCommand*>(obj);

  Context = cmd->context();

  Releaser->setContext(Context);
  Informer->setContext(Context);
  Launcher->setContext(Context);
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
  CheckTimer->setInterval(1000);

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
