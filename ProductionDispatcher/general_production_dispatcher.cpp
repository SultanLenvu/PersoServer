#include "general_production_dispatcher.h"
#include "Log/log_system.h"
#include "ProductionDispatcher/te310_printer.h"

GeneralProductionDispatcher::GeneralProductionDispatcher(const QString& name)
    : AbstractProductionDispatcher(name) {}

bool GeneralProductionDispatcher::checkConfiguration() {
  sendLog("Проверка конфигурации. ");

  if (!BoxStickerPrinter->checkConfiguration()) {
    sendLog(
        "Проверка конфигурации принтера для печати стикеров на боксы "
        "провалена. ");
    return false;
  }

  if (!PalletStickerPrinter->checkConfiguration()) {
    sendLog(
        "Проверка конфигурации принтера для печати стикеров на паллеты "
        "провалена. ");
    return false;
  }

  sendLog("Проверка конфигурации прошла успешно. ");
  return true;
}

void GeneralProductionDispatcher::start(ReturnStatus ret) {}

void GeneralProductionDispatcher::stop() {}

void GeneralProductionDispatcher::launchProductionLine(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::shutdownProductionLine(
    const StringDictionary& param,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::getProductionLineContext(
    const StringDictionary& param,
    StringDictionary& result,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::rollbackProductionLine(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::releaseTransponder(
    const StringDictionary& param,
    StringDictionary& seed,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::confirmTransponderRelease(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::rereleaseTransponder(
    const StringDictionary& param,
    StringDictionary& seed,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::confirmTransponderRerelease(
    const StringDictionary& param,
    StringDictionary& context,
    ReturnStatus ret) {}

void GeneralProductionDispatcher::printBoxStickerManually(
    const StringDictionary& param,
    ReturnStatus ret) {
  sendLog("Запуск печати стикера для бокса.");

  AbstractStickerPrinter::ReturnStatus status;
  status = BoxStickerPrinter->printBoxSticker(param);
}

void GeneralProductionDispatcher::printLastBoxStickerManually(
    ReturnStatus ret) {
  sendLog("Запуск печати последнего стикера для бокса.");

  AbstractStickerPrinter::ReturnStatus status;
  status = BoxStickerPrinter->printLastBoxSticker();
}

void GeneralProductionDispatcher::printPalletStickerManually(
    const StringDictionary& param,
    ReturnStatus ret) {
  sendLog("Запуск печати стикера для паллеты.");

  AbstractStickerPrinter::ReturnStatus status;
  status = PalletStickerPrinter->printPalletSticker(param);
}

void GeneralProductionDispatcher::printLastPalletStickerManually(
    ReturnStatus ret) {
  sendLog("Запуск печати последнего стикера для паллеты.");

  AbstractStickerPrinter::ReturnStatus status;
  status = PalletStickerPrinter->printLastPalletSticker();
}

void GeneralProductionDispatcher::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();

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
  if (LogEnable) {
    emit const_cast<GeneralProductionDispatcher*>(this)->logging(
        "GeneralProductionDispatcher - " + log);
  }
}

void GeneralProductionDispatcher::createStickerPrinters() {
#ifdef __linux__
  BoxStickerPrinter =
      new TE310Printer(this, BoxStickerPrinterIP, BoxStickerPrinterPort);
#else
  BoxStickerPrinter = std::unique_ptr<AbstractStickerPrinter>(
      new TE310Printer(BoxStickerPrinterName));
#endif /* __linux__ */
  connect(BoxStickerPrinter.get(), &AbstractStickerPrinter::logging,
          LogSystem::instance(), &LogSystem::generate);

#ifdef __linux__
  PalletStickerPrinter =
      new TE310Printer(this, PalletStickerPrinterIP, PalletStickerPrinterPort);
#else
  PalletStickerPrinter = std::unique_ptr<AbstractStickerPrinter>(
      new TE310Printer(PalletStickerPrinterName));
#endif /* __linux__ */
  connect(PalletStickerPrinter.get(), &AbstractStickerPrinter::logging,
          LogSystem::instance(), &LogSystem::generate);
}
