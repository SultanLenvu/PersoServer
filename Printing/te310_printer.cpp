#include <QHostAddress>

#include "te310_printer.h"

TE310Printer::TE310Printer(const QString& name) : AbstractStickerPrinter(name) {
  loadSetting();

  TscLib = std::unique_ptr<QLibrary>(new QLibrary(TscLibPath));
}

bool TE310Printer::init() {
  if (!loadTscLib()) {
    sendLog("Не удалось загрузить библиотеку.");
    return false;
  }

#ifdef __linux__
  if (!initConnectionByAddress()) {
    sendLog("Не удалось подключиться к принтеру.");
    return false;
  }
#else
  if (!initConnectionByName()) {
    sendLog("Не удалось подключиться к принтеру.");
    return false;
  }
#endif /* __linux__ */

  return true;
}

AbstractStickerPrinter::StickerPrinterType TE310Printer::type() {
  return TE310;
}

#ifdef __linux__
TE310Printer::TE310Printer(QObject* parent, const QHostAddress& ip, int port)
    : AbstractStickerPrinter(parent, TE310), IPAddress(ip) {
  setObjectName("TE310");
  Port = port;
  loadSetting();

  TscLib = new QLibrary(TscLibPath, this);
  loadTscLib();
}
#endif /* __linux__ */

ReturnStatus TE310Printer::printTransponderSticker(
    const StringDictionary& param) {
  if (!checkConfig()) {
    sendLog(QString("Не удалось подключиться к принтеру. Сброс"));
    return ReturnStatus::StickerPrinterConnectionError;
  }

  // Проврека параметров
  if (param.value("issuer_name").isEmpty() ||
      param.value("transponder_sn").isEmpty() ||
      param.value("transponder_pan").isEmpty()) {
    sendLog(QString("Получены некорректные параметры. Сброс."));
    return ReturnStatus::ParameterError;
  }
  sendLog(QString("Печать стикера транспондера для %1.")
              .arg(param.value("issuer_name")));

  if (!TscLib->isLoaded()) {
    sendLog("Библиотека не загружена. Сброс. ");
    return ReturnStatus::DynamicLibraryMissing;
  }

  // Сохраняем данные стикера
  LastTransponderSticker = param;

  if (param.value("issuer_name") == "Новое качество дорог") {
    printNkdSticker(param);
  } else if (param.value("issuer_name") == "Магистраль северной столицы") {
    printZsdSticker(param);
  } else {
    sendLog("Получено неизвестное название компании-эмитента. Сброс.");
    return ReturnStatus::ParameterError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TE310Printer::printLastTransponderSticker() {
  return printTransponderSticker(LastTransponderSticker);
}

ReturnStatus TE310Printer::printBoxSticker(const StringDictionary& param) {
  if (!checkConfig()) {
    sendLog(QString("Ошибка конфигурации. Сброс"));
    return ReturnStatus::StickerPrinterConnectionError;
  }

  if (param.value("box_id").isEmpty() ||
      param.value("transponder_model").isEmpty() ||
      param.value("box_assembled_units").isEmpty() ||
      param.value("first_transponder_sn").isEmpty() ||
      param.value("last_transponder_sn").isEmpty()) {
    sendLog(QString("Получены некорректные параметры. Сброс."));
    return ReturnStatus::ParameterError;
  }

  // Сохраняем данные о стикере
  LastBoxSticker = param;
  sendLog(QString("Печать стикера для бокса %1.").arg(param.value("id")));

#ifdef __linux__
  openEthernet(IPAddress.toString().toUtf8().data(), Port);
#else
  openPort(objectName().toUtf8().data());
#endif /* __linux__ */
  sendCommand("SIZE 100 mm, 50 mm");
  sendCommand("GAP 2 mm,2 mm");
  sendCommand("REFERENCE 0,0");
  sendCommand("DIRECTION 1");
  sendCommand("CLS");
  sendCommand("TEXT 600,10,\"D.FNT\",0,2,2,2,\"JSC PowerSyntez\"");
  sendCommand("BOX 25, 50, 1150, 560, 4 ");
  sendCommand("TEXT 50, 90, \"D.FNT\", 0, 2, 2, 1,\"MODEL:\"");
  sendCommand(QString("TEXT 600, 90, \"D.FNT\", 0, 2, 2, 1, \"%1\"")
                  .arg(param.value("transponder_model"))
                  .toUtf8()
                  .data());
  /* На случай если модель будет заменена на артикул */
  //  sendCommand("TEXT 50, 90, \"D.FNT\", 0, 2, 2, 1,\"ARTICLE NO:\"");
  //  sendCommand(QString("BARCODE 600, 75, \"128M\", 50, 2, 0, 2, 4, 1,
  //  \"%1\"")
  //                  .arg(param.value("transponder_model"))
  //                  .toUtf8()
  //                  .data());
  sendCommand("TEXT 50, 190, \"D.FNT\", 0, 2, 2, 1, \"QUANTITY:\"");
  sendCommand(QString("TEXT 600, 190, \"D.FNT\", 0, 2, 2, 1, \"%1\"")
                  .arg(param.value("box_assembled_units"))
                  .toUtf8()
                  .data());
  sendCommand("TEXT 50, 290, \"D.FNT\", 0, 2, 2, 1, \"SERIAL NO FROM:\"");
  sendCommand(QString("BARCODE 600, 275, \"128M\", 50, 2, 0, 2, 4, 1, \"%1\"")
                  .arg(param.value("first_transponder_sn"))
                  .toUtf8()
                  .data());
  sendCommand("TEXT 50, 390, \"D.FNT\", 0, 2, 2, 1, \"SERIAL NO TO:\"");
  sendCommand(QString("BARCODE 600, 375, \"128M\", 50, 2, 0, 2, 4, 1, \"%1\"")
                  .arg(param.value("last_transponder_sn"))
                  .toUtf8()
                  .data());
  sendCommand("TEXT 50, 490, \"D.FNT\", 0, 2, 2, 1, \"BOX NO:\"");
  sendCommand(QString("BARCODE 600, 475, \"128M\", 50, 2, 0, 2, 4, 1, \"%1\"")
                  .arg(param.value("id"))
                  .toUtf8()
                  .data());
  sendCommand("PRINT 1");
  closePort();

  return ReturnStatus::NoError;
}

ReturnStatus TE310Printer::printLastBoxSticker() {
  return printBoxSticker(LastBoxSticker);
}

ReturnStatus TE310Printer::printPalletSticker(const StringDictionary& param) {
  if (!checkConfig()) {
    sendLog(QString("Ошибка конфигурации. Сброс"));
    return ReturnStatus::StickerPrinterConnectionError;
  }

  if (param.value("id").isEmpty() ||
      param.value("transponder_model").isEmpty() ||
      param.value("quantity").isEmpty() ||
      param.value("first_box_id").isEmpty() ||
      param.value("last_box_id").isEmpty() ||
      param.value("assembly_date").isEmpty()) {
    sendLog(QString("Получены некорректные параметры. Сброс."));
    return ReturnStatus::ParameterError;
  }

  // Сохраняем данные о стикере
  LastPalletSticker = param;

  sendLog(QString("Печать стикера для паллеты %1.").arg(param.value("id")));

#ifdef __linux__
  openEthernet(IPAddress.toString().toUtf8().data(), Port);
#else
  openPort(objectName().toUtf8().data());
#endif /* __linux__ */

  sendCommand("SIZE 100 mm,100 mm");
  sendCommand("GAP 2 mm,2 mm");
  sendCommand("REFERENCE 0,0");
  sendCommand("DIRECTION 1");
  sendCommand("CLS");
  sendCommand("TEXT 600, 25, \"D.FNT\", 0, 3, 3, 2, \"JSC PowerSyntez\"");
  sendCommand("BOX 25, 100, 1150, 1075, 4");
  sendCommand("TEXT 50, 150, \"D.FNT\", 0, 3, 3, 1,\"MODEL:\"");
  sendCommand(QString("TEXT 600, 150,\"D.FNT\", 0, 3, 3, 1, \"%1\"")
                  .arg(param.value("transponder_model"))
                  .toUtf8()
                  .constData());

  sendCommand("TEXT 50, 300, \"D.FNT\", 0, 3, 3, 1, \"QUANTITY:\"");
  sendCommand(QString("TEXT 600, 300, \"D.FNT\", 0, 3, 3, 1, \"%1\"")
                  .arg(param.value("quantity"))
                  .toUtf8()
                  .constData());

  sendCommand("TEXT 50, 450, \"D.FNT\", 0, 3, 3, 1, \"BOX NO FROM:\"");
  sendCommand(QString("BARCODE 600, 425, \"128M\", 75, 2, 0, 3, 6, 1, \"%1\"")
                  .arg(param.value("first_box_id"))
                  .toUtf8()
                  .constData());

  sendCommand("TEXT 50, 600, \"D.FNT\", 0, 3, 3, 1, \"BOX NO TO:\"");
  sendCommand(QString("BARCODE 600, 575, \"128M\", 75, 2, 0, 3, 6, 1, \"%1\"")
                  .arg(param.value("last_box_id"))
                  .toUtf8()
                  .constData());

  sendCommand("TEXT 50, 750, \"D.FNT\", 0, 3, 3, 1, \"PALLET NO:\"");
  sendCommand(QString("BARCODE 600, 725, \"128M\", 75, 2, 0, 3, 6, 1, \"%1\"")
                  .arg(param.value("id"))
                  .toUtf8()
                  .constData());

  sendCommand("TEXT 50, 900, \"D.FNT\", 0, 3, 3, 1,\"ASSEMBLY DATE:\"");
  sendCommand(QString("TEXT 600, 900, \"D.FNT\", 0, 3, 3, 1, \"%1\"")
                  .arg(param.value("assembly_date"))
                  .toUtf8()
                  .constData());

  sendCommand("TEXT 600, 1090, \"D.FNT\", 0, 3, 3, 2, \"Made in Russia\"");
  sendCommand("PRINT 1");

  closePort();

  return ReturnStatus::NoError;
}

ReturnStatus TE310Printer::printLastPalletSticker() {
  return printPalletSticker(LastPalletSticker);
}

ReturnStatus TE310Printer::exec(const QStringList& commandScript) {
  if (!checkConfig()) {
    sendLog(QString("Не удалось подключиться к принтеру. Сброс"));
    return ReturnStatus::StickerPrinterConnectionError;
  }

#ifdef __linux__
  openEthernet(IPAddress.toString().toUtf8().data(), Port);
  openPort(objectName().toUtf8().data());
#else
  openPort(objectName().toUtf8().data());
#endif /* __linux__ */
  for (int32_t i = 0; i < commandScript.size(); i++) {
    sendCommand(commandScript.at(i).toUtf8().data());
  }
  closePort();

  return ReturnStatus::NoError;
}

void TE310Printer::applySetting() {
  sendLog("Применение новых настроек.");

  loadSetting();

  init();
}

void TE310Printer::loadSetting() {
  QSettings settings;

  TscLibPath = settings.value("te310_printer/library_path").toString();
}

void TE310Printer::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
}

bool TE310Printer::loadTscLib() {
  if (!TscLib->load()) {
    about = nullptr;
    openPort = nullptr;
#ifdef __linux__
    openEthernet = nullptr;
#endif /* __linux__ */
    sendCommand = nullptr;
    closePort = nullptr;

    return false;
  }
  about = (TscAbout)TscLib->resolve("about");
  openPort = (TscOpenPort)TscLib->resolve("openport");
#ifdef __linux__
  openEthernet = (TscOpenEthernet)TscLib->resolve("openethernet");
#endif /* __linux__ */
  sendCommand = (TscSendCommand)TscLib->resolve("sendcommand");
  closePort = (TscClosePort)TscLib->resolve("closeport");

  return true;
}

bool TE310Printer::initConnectionByName() {
  QList<QString> printers = QPrinterInfo::availablePrinterNames();
  if (std::find_if(printers.begin(), printers.end(), [this](const QString p) {
        return p == objectName();
      }) == printers.end()) {
    sendLog("Не найден драйвер операционной системы. ");
    return false;
  }
  if (QPrinterInfo::printerInfo(objectName()).state() == QPrinter::Error) {
    sendLog("Недоступен.");
    return false;
  }

  if (openPort(objectName().toUtf8().constData()) == 0) {
    sendLog("Недоступен.");
    return false;
  }
  closePort();

  sendLog("Соединение установлено. ");
  return true;
}

bool TE310Printer::checkConfig() {
  if (!TscLib->isLoaded()) {
    sendLog("Библиотека не загружена. ");
    return false;
  }

#ifdef __linux__
  if (openEthernet(IPAddress.toString().toUtf8().data(), Port) == 0) {
    sendLog("Недоступен.");
    return false;
  }
#else
  if (openPort(objectName().toUtf8().constData()) == 0) {
    sendLog("Недоступен.");
    return false;
  }
#endif /* __linux__ */

  return true;
}

#ifdef __linux__
bool TE310Printer::initConnectionByAddress() {
  int32_t result = openEthernet(IPAddress.toString().toUtf8().data(), Port);
  if (result == 0) {
    sendLog("Недоступен.");
    return false;
  }
  closePort();

  sendLog("Соединение установлено. ");
  return true;
}
#endif /* __linux__ */

void TE310Printer::printNkdSticker(const StringDictionary& param) {
#ifdef __linux__
  openEthernet(IPAddress.toString().toUtf8().data(), Port);
#else
  openPort(objectName().toUtf8().data());
#endif /* __linux__ */
  sendCommand("SIZE 27 mm, 27 mm");
  sendCommand("GAP 2 mm,2 mm");
  sendCommand("REFERENCE 0,0");
  sendCommand("DIRECTION 1");
  sendCommand("CLS");
  sendCommand(QString("TEXT 162,30,\"D.FNT\",0,1,1,2,\"PAN: %1\"")
                  .arg(param.value("pan"))
                  .toUtf8()
                  .data());
  sendCommand(QString("QRCODE "
                      "60,60,H,10,A,0,X204,J1,M2,\"%1\n\r%2\"")
                  .arg(param.value("pan"), param.value("sn"))
                  .toUtf8()
                  .data());
  sendCommand(QString("TEXT 162,276,\"D.FNT\",0,1,1,2,\"SN: %1\"")
                  .arg(param.value("sn"))
                  .toUtf8()
                  .data());
  sendCommand("PRINT 1");
  closePort();
}

void TE310Printer::printZsdSticker(const StringDictionary& param) {
#ifdef __linux__
  openEthernet(IPAddress.toString().toUtf8().data(), Port);
#else
  openPort(objectName().toUtf8().data());
#endif /* __linux__ */
  sendCommand("SIZE 30 mm, 20 mm");
  sendCommand("GAP 2 mm, 1 mm");
  sendCommand("DIRECTION 1");
  sendCommand("CLS");
  sendCommand(QString("TEXT 180,12,\"D.FNT\",0,1,1,2,\"SN: %1\"")
                  .arg(param.value("sn"))
                  .toUtf8()
                  .data());
  sendCommand(QString("BARCODE 18,36,\"128\",144,2,0,2,2,\"%1\"")
                  .arg(param.value("pan"))
                  .toUtf8()
                  .data());
  sendCommand("PRINT 1");
  closePort();
}
