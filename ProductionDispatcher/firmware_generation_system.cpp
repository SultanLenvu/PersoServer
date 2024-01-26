#include "firmware_generation_system.h"
#include "definitions.h"
#include "des.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(const QString& name)
    : AbstractFirmwareGenerationSystem(name) {
  FirmwareBaseFile = std::unique_ptr<QFile>(new QFile());
  FirmwareDataFile = std::unique_ptr<QFile>(new QFile());

  // Загружаем настройки
  loadSettings();
}

FirmwareGenerationSystem::~FirmwareGenerationSystem() {}

bool FirmwareGenerationSystem::init() {
  uint32_t currentSize = 0;
  uint32_t bytesToAdd = 0;
  QByteArray padding;

  if (FirmwareBaseFile->open(QIODevice::ReadWrite)) {
    currentSize = FirmwareBaseFile->size();
    if (currentSize < FIRMWARE_BASE_SIZE) {
      bytesToAdd = FIRMWARE_BASE_SIZE - currentSize;
      padding = QByteArray(bytesToAdd, '\xFF');

      FirmwareBaseFile->resize(FIRMWARE_BASE_SIZE);
      FirmwareBaseFile->seek(currentSize);
      FirmwareBaseFile->write(padding);
    }
    FirmwareBaseFile->close();
  } else {
    sendLog("Не удалось открыть базовый файл прошивки на запись.");
    return false;
  }

  if (FirmwareDataFile->open(QIODevice::ReadWrite)) {
    currentSize = FirmwareDataFile->size();
    if (currentSize < FIRMWARE_DATA_SIZE) {
      bytesToAdd = FIRMWARE_DATA_SIZE - currentSize;
      padding = QByteArray(bytesToAdd, '\xFF');

      FirmwareDataFile->resize(FIRMWARE_DATA_SIZE);
      FirmwareDataFile->seek(currentSize);
      FirmwareDataFile->write(padding);
    }
    FirmwareDataFile->close();
  } else {
    sendLog("Не удалось открыть файл данных прошивки на запись.");
    return false;
  }

  sendLog("Инициализация успешно заврешена.");
  return true;
}

ReturnStatus FirmwareGenerationSystem::generate(const StringDictionary& seed,
                                                QByteArray& assembledFirmware) {
  ReturnStatus ret;
  QByteArray firmwareData;

  // Подгототавливаем память
  assembledFirmware.clear();
  assembledFirmware.reserve(FIRMWARE_SIZE);

  ret = generateFirmwareData(seed, firmwareData);
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при генерации данных прошивки. ");
    return ret;
  }

  ret = assembleFirmware(firmwareData, assembledFirmware);
  if (ret != ReturnStatus::NoError) {
    sendLog("Получена ошибка при сборке прошивки из базы и данных. ");
    return ret;
  }

  return ReturnStatus::NoError;
}

void FirmwareGenerationSystem::loadSettings() {
  QSettings settings;

  FirmwareBaseFile->setFileName(
      settings.value("firmware_generation_system/firmware_base_path")
          .toString());
  FirmwareDataFile->setFileName(
      settings.value("firmware_generation_system/firmware_data_path")
          .toString());
}

void FirmwareGenerationSystem::sendLog(const QString& log) const {
  emit const_cast<FirmwareGenerationSystem*>(this)->logging(objectName() +
                                                            " - " + log);
}

ReturnStatus FirmwareGenerationSystem::assembleFirmware(
    const QByteArray& firmwareData,
    QByteArray& assembledFirmware) {
  if (!FirmwareBaseFile->open(QIODevice::ReadOnly)) {
    sendLog("Не удалось открыть файл прошивки на чтение.");
    return ReturnStatus::FileOpenError;
  }

  // Загружаем содержимое шаблонного файла базы прошивки
  assembledFirmware.append(FirmwareBaseFile->readAll());

  // Закрываем файл
  FirmwareBaseFile->close();

  // Сцепляем базу и данные прошивки
  assembledFirmware.append(firmwareData);

  return ReturnStatus::NoError;
}

ReturnStatus FirmwareGenerationSystem::generateFirmwareData(
    const StringDictionary& seed,
    QByteArray& firmwareData) {
  if (!FirmwareDataFile->open(QIODevice::ReadOnly)) {
    sendLog("Не удалось открыть файл прошивки на чтение.");
    return ReturnStatus::FileOpenError;
  }

  // Загружаем содержимое шаблонного файла данных прошивки
  firmwareData = FirmwareDataFile->readAll();

  // EFC атрибуты
  firmwareData.replace(
      EFC_CONTEXT_MARK_FPI, EFC_CONTEXT_MARK_SIZE,
      QByteArray::fromHex(seed.value("efc_context_mark").toUtf8()));

  QString paymentMeans;
  generatePaymentMeans(seed.value("personal_account_number"), paymentMeans);
  firmwareData.replace(PAYMENT_MEANS_FPI, PAYMENT_MEANS_SIZE,
                       QByteArray::fromHex(paymentMeans.toUtf8()));

  QString equipmnetObuId =
      QString("%1").arg(QString::number(seed.value("id").toInt(), 16),
                        EQUIPMENT_OBU_ID_SIZE * 2, QChar('0'));
  firmwareData.replace(EQUIPMENT_OBU_ID_FPI, EQUIPMENT_OBU_ID_SIZE,
                       QByteArray::fromHex(equipmnetObuId.toUtf8()));

  // Атрибуты производителя
  firmwareData.replace(
      MANUFACTURER_ID_FPI, MANUFACTURER_ID_SIZE,
      QByteArray::fromHex(seed.value("manufacturer_id").toUtf8()));

  firmwareData.replace(MANUFACTURING_SERIAL_NO_FPI,
                       MANUFACTURING_SERIAL_NO_SIZE,
                       QByteArray::fromHex(equipmnetObuId.toUtf8()));

  firmwareData.replace(
      EQUIPMENT_CLASS_FPI, EQUIPMENT_CLASS_SIZE,
      QByteArray::fromHex(seed.value("equipment_class").toUtf8()));

  QByteArray transponderPartNo =
      QString("%1")
          .arg(seed.value("transponder_model"))
          .toUtf8()
          .leftJustified(TRANSPONDER_PART_NO_SIZE, '\x00');
  firmwareData.replace(TRANSPONDER_PART_NO_FPI, TRANSPONDER_PART_NO_SIZE,
                       transponderPartNo);

  firmwareData.replace(
      ACCR_REFERENCE_FPI, ACCR_REFERENCE_SIZE,
      QByteArray::fromHex(seed.value("accr_reference").toUtf8()));

  firmwareData.replace(
      BATTERY_INSERTATION_DATE_FPI, BATTERY_INSERTATION_DATE_SIZE,
      QByteArray::fromHex(seed.value("battery_insertation_date").toUtf8()));

  // Ключи безопасности
  generateCommonKeys(seed);
  firmwareData.replace(AUKEY1_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key1"));
  firmwareData.replace(AUKEY2_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key2"));
  firmwareData.replace(AUKEY3_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key3"));
  firmwareData.replace(AUKEY4_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key4"));
  firmwareData.replace(AUKEY5_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key5"));
  firmwareData.replace(AUKEY6_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key6"));
  firmwareData.replace(AUKEY7_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key7"));
  firmwareData.replace(AUKEY8_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("au_key8"));
  firmwareData.replace(ACCRKEY_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("accr_key"));
  firmwareData.replace(PERKEY_FPI, COMMON_KEY_SIZE,
                       CommonKeys.value("per_key"));

  // Логи
  firmwareData.replace(TRANSPONDER_LOG_FPI, TRANSPONDER_LOG_SIZE,
                       QByteArray(TRANSPONDER_LOG_SIZE, '\x00'));

  FirmwareDataFile->close();
  return ReturnStatus::NoError;
}

void FirmwareGenerationSystem::generateCommonKeys(
    const StringDictionary& seed) {
  // Создаем инициализаторы
  QByteArray accrInit;
  QByteArray accrReference = seed.value("accr_reference").toUtf8();
  for (uint32_t i = 0; i < 4; i++) {
    accrInit.append(QByteArray::fromHex(accrReference));
  }

  QByteArray auInit;
  QByteArray ecm = QByteArray::fromHex(seed.value("efc_context_mark").toUtf8());
  QByteArray pan =
      QByteArray::fromHex(seed.value("personal_account_number").toUtf8());
  QByteArray compactPan;

  for (uint32_t i = 0; i < 4; i++) {
    compactPan.append(pan.at(i) ^ pan.at(i + 4));
  }

  auInit.append(compactPan);
  auInit.append(ecm.at(0));
  auInit.append(ecm.at(1));
  auInit.append(ecm.at(2));
  auInit.append(static_cast<char>(0x00));

  // Генерируем ключи
  uint8_t* init;
  QByteArray masterKeyValue;
  for (QHash<QString, QString>::const_iterator it = seed.begin();
       it != seed.end(); it++) {
    uint8_t* result = new uint8_t[COMMON_KEY_SIZE];

    if ((it.key() == "accr_key") || (it.key() == "per_key")) {
      init = reinterpret_cast<uint8_t*>(accrInit.data());
    } else {
      init = reinterpret_cast<uint8_t*>(auInit.data());
    }
    masterKeyValue = QByteArray::fromHex(it.value().toUtf8());
    TDES::EDE2_encryption(
        init, reinterpret_cast<const uint8_t*>(masterKeyValue.data()),
        reinterpret_cast<const uint8_t*>(masterKeyValue.data() + 8), result);

    CommonKeys.insert(it.key(),
                      QByteArray::fromRawData(reinterpret_cast<char*>(result),
                                              COMMON_KEY_SIZE));
  }
}

void FirmwareGenerationSystem::generatePaymentMeans(const QString& pan,
                                                    QString& paymentMeans) {
  QDate currentDate = QDate::currentDate();
  QString expirationDate;

  if (currentDate.year() > 2117) {
    expirationDate = "0000";
  } else {
    /* DateCompact format yyyyyyymmmmddddd (bits) */
    uint16_t dateCompactNumber = 0;
    dateCompactNumber |=
        ((static_cast<uint16_t>(currentDate.year()) - 1990 + 10) << 9);
    /* Вермя экспирации PAN'a + 10 лет с момента выпуска */
    dateCompactNumber |= (static_cast<uint16_t>(currentDate.month()) << 5);
    dateCompactNumber |= static_cast<uint16_t>(currentDate.day());

    expirationDate =
        QString::number(dateCompactNumber, 16).leftJustified(4, '0');
  }

  paymentMeans = pan + expirationDate + "0000";
}
