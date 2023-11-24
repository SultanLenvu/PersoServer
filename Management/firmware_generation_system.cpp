#include "firmware_generation_system.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(QObject *parent) : QObject(parent)
{
  setObjectName("FirmwareGenerationSystem");

  FirmwareBaseFile = new QFile(this);
  FirmwareDataFile = new QFile(this);

  // Загружаем настройки
  loadSettings();
}

FirmwareGenerationSystem::~FirmwareGenerationSystem() {}

bool FirmwareGenerationSystem::generate(const QHash<QString, QString>* seed,
                                        QByteArray* assembledFirmware) {
  QByteArray firmwareData;

  if (!generateFirmwareData(seed, &firmwareData)) {
    sendLog("Получена ошибка при генерации данных прошивки. ");
    return false;
  }

  if (!assembleFirmware(&firmwareData, assembledFirmware)) {
    sendLog("Получена ошибка при сборке прошивки из базы и данных. ");
    return false;
  }

  return true;
}

void FirmwareGenerationSystem::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();

  FirmwareBaseFile->setFileName(
      settings.value("firmware_generation_system/firmware_base_path")
          .toString());
  FirmwareDataFile->setFileName(
      settings.value("firmware_generation_system/firmware_data_path")
          .toString());

  // Дополнение файлов
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
    return;
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
    sendLog("Не удалось открыть базовый файл прошивки на запись.");
    return;
  }
}

void FirmwareGenerationSystem::sendLog(const QString& log) const {
  if (LogEnable) {
    emit logging("FirmwareGenerationSystem - " + log);
  }
}

bool FirmwareGenerationSystem::assembleFirmware(const QByteArray* firmwareData,
                                                QByteArray* assembledFirmware) {
  if (!FirmwareBaseFile->open(QIODevice::ReadOnly)) {
    sendLog("Не удалось открыть файл прошивки на чтение.");
    return false;
  }

  // Загружаем содержимое шаблонного файла базы прошивки
  assembledFirmware->append(FirmwareBaseFile->readAll());

  // Закрываем файл
  FirmwareBaseFile->close();

  // Сцепляем базу и данные прошивки
  assembledFirmware->append(*firmwareData);

  return true;
}

bool FirmwareGenerationSystem::generateFirmwareData(
    const QHash<QString, QString>* transponderSeed,
    QByteArray* firmwareData) {
  if (!FirmwareDataFile->open(QIODevice::ReadOnly)) {
    sendLog("Не удалось открыть файл прошивки на чтение.");
    return false;
  }

  // Загружаем содержимое шаблонного файла данных прошивки
  *firmwareData = FirmwareDataFile->readAll();

  // EFC атрибуты
  firmwareData->replace(
      EFC_CONTEXT_MARK_FPI, EFC_CONTEXT_MARK_SIZE,
      QByteArray::fromHex(transponderSeed->value("efc_context_mark").toUtf8()));

  QString paymentMeans;
  generatePaymentMeans(transponderSeed->value("personal_account_number"),
                       paymentMeans);
  firmwareData->replace(PAYMENT_MEANS_FPI, PAYMENT_MEANS_SIZE,
                        QByteArray::fromHex(paymentMeans.toUtf8()));

  QString equipmnetObuId = QString("%1").arg(
      QString::number(transponderSeed->value("id").toInt(), 16),
      EQUIPMENT_OBU_ID_SIZE * 2, QChar('0'));
  firmwareData->replace(EQUIPMENT_OBU_ID_FPI, EQUIPMENT_OBU_ID_SIZE,
                        QByteArray::fromHex(equipmnetObuId.toUtf8()));

  // Атрибуты производителя
  firmwareData->replace(
      MANUFACTURER_ID_FPI, MANUFACTURER_ID_SIZE,
      QByteArray::fromHex(transponderSeed->value("manufacturer_id").toUtf8()));

  firmwareData->replace(MANUFACTURING_SERIAL_NO_FPI,
                        MANUFACTURING_SERIAL_NO_SIZE,
                        QByteArray::fromHex(equipmnetObuId.toUtf8()));

  firmwareData->replace(
      EQUIPMENT_CLASS_FPI, EQUIPMENT_CLASS_SIZE,
      QByteArray::fromHex(transponderSeed->value("equipment_class").toUtf8()));

  QByteArray transponderPartNo =
      QString("%1")
          .arg(transponderSeed->value("transponder_model"))
          .toUtf8()
          .leftJustified(TRANSPONDER_PART_NO_SIZE, '\x00');
  firmwareData->replace(TRANSPONDER_PART_NO_FPI, TRANSPONDER_PART_NO_SIZE,
                        transponderPartNo);

  firmwareData->replace(
      ACCR_REFERENCE_FPI, ACCR_REFERENCE_SIZE,
      QByteArray::fromHex(transponderSeed->value("accr_reference").toUtf8()));

  firmwareData->replace(
      BATTERY_INSERTATION_DATE_FPI, BATTERY_INSERTATION_DATE_SIZE,
      QByteArray::fromHex(
          transponderSeed->value("battery_insertation_date").toUtf8()));

  // Ключи безопасности
  generateCommonKeys(transponderSeed);
  firmwareData->replace(AUKEY1_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key1"));
  firmwareData->replace(AUKEY2_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key2"));
  firmwareData->replace(AUKEY3_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key3"));
  firmwareData->replace(AUKEY4_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key4"));
  firmwareData->replace(AUKEY5_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key5"));
  firmwareData->replace(AUKEY6_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key6"));
  firmwareData->replace(AUKEY7_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key7"));
  firmwareData->replace(AUKEY8_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("au_key8"));
  firmwareData->replace(ACCRKEY_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("accr_key"));
  firmwareData->replace(PERKEY_FPI, COMMON_KEY_SIZE,
                        CommonKeys.value("per_key"));

  // Логи
  firmwareData->replace(TRANSPONDER_LOG_FPI, TRANSPONDER_LOG_SIZE,
                        QByteArray(TRANSPONDER_LOG_SIZE, '\x00'));

  FirmwareDataFile->close();
  return true;
}

void FirmwareGenerationSystem::generateCommonKeys(
    const QHash<QString, QString>* transpoderSeed) {
  // Создаем инициализаторы
  QByteArray accrInit;
  QByteArray accrReference = transpoderSeed->value("accr_reference").toUtf8();
  for (uint32_t i = 0; i < 4; i++) {
    accrInit.append(QByteArray::fromHex(accrReference));
  }

  QByteArray auInit;
  QByteArray ecm =
      QByteArray::fromHex(transpoderSeed->value("efc_context_mark").toUtf8());
  QByteArray pan = QByteArray::fromHex(
      transpoderSeed->value("personal_account_number").toUtf8());
  QByteArray compactPan;

  for (uint32_t i = 0; i < 4; i++) {
    compactPan.append(pan.at(i) ^ pan.at(i + 4));
  }

  auInit.append(compactPan);
  auInit.append(ecm.at(0));
  auInit.append(ecm.at(1));
  auInit.append(ecm.at(2));
  auInit.append(static_cast<uint8_t>(0x00));

  // Генерируем ключи
  uint8_t* init;
  QByteArray masterKeyValue;
  for (QHash<QString, QString>::const_iterator it = transpoderSeed->begin();
       it != transpoderSeed->end(); it++) {
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
    delete[] result;
  }
}

void FirmwareGenerationSystem::generatePaymentMeans(const QString& pan,
                                                    QString& paymentMeans) {
  QDate currentDate = QDate::currentDate();
  QString expirationDate;

  if (currentDate.year() > 2117) {
    expirationDate = "0000";
  } else {
    /* DateCompact format yyyyyyymmmmddddd */
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
