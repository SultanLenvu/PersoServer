#include "firmware_generation_system.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(QObject *parent) : QObject(parent)
{
  setObjectName("FirmwareGenerationSystem");

  FirmwareBase = new QFile(this);
  FirmwareData = new QFile(this);

  // Загружаем настройки
  loadSettings();
}

void FirmwareGenerationSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
}

bool FirmwareGenerationSystem::generate(const TransponderSeedModel* seed,
                                        QByteArray* firmware) {
  if (!FirmwareData->open(QIODevice::ReadOnly)) {
    emit logging("Не удалось открыть файл прошивки на чтение.");
    return false;
  }

  // Память не освобождать, т.к. она используется далее в QByteArray
  char* buffer = new char[FIRMWARE_DATA_SIZE];

  QDataStream in(FirmwareData);
  in.readRawData(buffer, FIRMWARE_DATA_SIZE);
  *firmware = QByteArray::fromRawData(buffer, FIRMWARE_DATA_SIZE);

  // EFC атрибуты
  firmware->replace(
      EFC_CONTEXT_MARK_FPI, EFC_CONTEXT_MARK_SIZE,
      QByteArray::fromHex(
          seed->attributes()->value("efc_context_mark").toUtf8()));

  QString paymentMeans;
  generatePaymentMeans(seed->attributes()->value("personal_account_number"),
                       paymentMeans);
  firmware->replace(PAYMENT_MEANS_FPI, PAYMENT_MEANS_SIZE,
                    QByteArray::fromHex(paymentMeans.toUtf8()));

  firmware->replace(
      EQUIPMENT_OBU_ID_FPI, EQUIPMENT_OBU_ID_SIZE,
      QByteArray::fromHex(
          QString::number(seed->attributes()->value("id").toInt(), 16)
              .toUtf8()));

  // Атрибуты производителя
  firmware->replace(MANUFACTURER_ID_FPI, MANUFACTURER_ID_SIZE,
                    QByteArray::fromHex(
                        seed->attributes()->value("manufacturer_id").toUtf8()));

  firmware->replace(
      MANUFACTURING_SERIAL_NO_FPI, MANUFACTURING_SERIAL_NO_SIZE,
      QByteArray::fromHex(
          QString::number(seed->attributes()->value("id").toInt(), 16)
              .toUtf8()));

  firmware->replace(EQUIPMENT_CLASS_FPI, EQUIPMENT_CLASS_SIZE,
                    QByteArray::fromHex(
                        seed->attributes()->value("equipment_class").toUtf8()));

  firmware->replace(
      TRANSPONDER_PART_NO_FPI, TRANSPONDER_PART_NO_SIZE,
      QByteArray::fromHex(
          seed->attributes()->value("transponder_model").toUtf8()));

  firmware->replace(ACCR_REFERENCE_FPI, ACCR_REFERENCE_SIZE,
                    QByteArray::fromHex(
                        seed->attributes()->value("accr_reference").toUtf8()));

  QDate date = QDate::currentDate();
  QString batteryInsertationDate = QString("%1%2")
                                       .arg(date.weekNumber(), 2, QChar('0'))
                                       .arg(QString::number(date.year() % 100));
  firmware->replace(BATTERY_INSERTATION_DATE_FPI, BATTERY_INSERTATION_DATE_SIZE,
                    QByteArray::fromHex(batteryInsertationDate.toUtf8()));

  // Ключи безопасности
  generateCommonKeys(seed);
  firmware->replace(835, COMMON_KEY_SIZE, CommonKeys.value("au_key1"));
  firmware->replace(AUKEY2_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key2"));
  firmware->replace(AUKEY3_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key3"));
  firmware->replace(AUKEY4_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key4"));
  firmware->replace(AUKEY5_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key5"));
  firmware->replace(AUKEY6_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key6"));
  firmware->replace(AUKEY7_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key7"));
  firmware->replace(AUKEY8_FPI, COMMON_KEY_SIZE, CommonKeys.value("au_key8"));
  firmware->replace(ACCRKEY_FPI, COMMON_KEY_SIZE, CommonKeys.value("accr_key"));
  firmware->replace(PERKEY_FPI, COMMON_KEY_SIZE, CommonKeys.value("per_key"));

  FirmwareData->close();
  return true;
}

void FirmwareGenerationSystem::loadSettings() {
  QSettings settings;

  FirmwareBase->setFileName(settings.value("Firmware/Base/Path").toString());
  FirmwareData->setFileName(settings.value("Firmware/Data/Path").toString());

  // Дополнение файлов
  uint32_t currentSize = 0;
  uint32_t bytesToAdd = 0;
  QByteArray padding;
  if (FirmwareBase->open(QIODevice::ReadWrite)) {
    currentSize = FirmwareBase->size();
    if (currentSize < FIRMWARE_BASE_SIZE) {
      bytesToAdd = FIRMWARE_BASE_SIZE - currentSize;
      padding = QByteArray(bytesToAdd, '\xFF');

      FirmwareBase->resize(FIRMWARE_BASE_SIZE);
      FirmwareBase->seek(currentSize);
      FirmwareBase->write(padding);
    }
    FirmwareBase->close();
  } else {
    emit logging("Не удалось открыть базовый файл прошивки на запись.");
    return;
  }

  if (FirmwareData->open(QIODevice::ReadWrite)) {
    currentSize = FirmwareData->size();
    if (currentSize < FIRMWARE_DATA_SIZE) {
      bytesToAdd = FIRMWARE_DATA_SIZE - currentSize;
      padding = QByteArray(bytesToAdd, '\xFF');

      FirmwareData->resize(FIRMWARE_DATA_SIZE);
      FirmwareData->seek(currentSize);
      FirmwareData->write(padding);
    }
    FirmwareData->close();
  } else {
    emit logging("Не удалось открыть базовый файл прошивки на запись.");
    return;
  }
}

void FirmwareGenerationSystem::generateCommonKeys(
    const TransponderSeedModel* seed) {
  // Создаем инициализаторы
  QByteArray accrInit;
  QByteArray accrReference =
      seed->attributes()->value("accr_reference").toUtf8();
  for (uint32_t i = 0; i < 4; i++) {
    accrInit.append(QByteArray::fromHex(accrReference));
  }

  QByteArray auInit;
  QByteArray ecm = seed->attributes()->value("efc_context_mark").toUtf8();
  QByteArray pan =
      seed->attributes()->value("personal_account_number").toUtf8();
  QByteArray compactPan;

  for (uint32_t i = 0; i < 4; i++) {
    compactPan.append(pan.at(i) ^ pan.at(i + 4));
  }

  auInit.append(compactPan);
  auInit.append(ecm.at(0));
  auInit.append(ecm.at(1));
  auInit.append(ecm.at(2));
  auInit.append(static_cast<uint8_t>(0x00));

  // Преобразуем мастер ключи из сида
  for (QMap<QString, QString>::const_iterator it =
           seed->masterKeys()->constBegin();
       it != seed->masterKeys()->constEnd(); it++) {
    MasterKeys.insert(it.key(), QByteArray::fromHex(it.value().toUtf8()));
    CommonKeys.insert(it.key(), QByteArray(8, '\xFF'));
  }

  // Генерируем ключи
  uint8_t* init;
  for (QMap<QString, QByteArray>::iterator it = MasterKeys.begin();
       it != MasterKeys.end(); it++) {
    uint8_t* result = new uint8_t[COMMON_KEY_SIZE];

    if ((it.key() == "accr_key") || (it.key() == "per_key")) {
      init = reinterpret_cast<uint8_t*>(accrInit.data());
    } else {
      init = reinterpret_cast<uint8_t*>(auInit.data());
    }
    TDES::EDE2_encryption(init, reinterpret_cast<uint8_t*>(it.value().data()),
                          reinterpret_cast<uint8_t*>(it.value().data() + 8),
                          result);

    //    CommonKeys.insert(it.key(),
    //                      QByteArray::fromRawData(reinterpret_cast<char*>(result),
    //                                              COMMON_KEY_SIZE));
  }
}

void FirmwareGenerationSystem::generatePaymentMeans(const QString& pan,
                                                    QString& paymentMeans) {
  QDate currentDate = QDate::currentDate();
  QString expirationDate;

  if (currentDate.year() > 2117) {
    expirationDate = "0000";
  } else {
    uint16_t dateCompactNumber = 0;
    dateCompactNumber |= static_cast<uint16_t>(currentDate.year()) - 1990 + 10;
    dateCompactNumber |= (static_cast<uint16_t>(currentDate.month()) << 7);
    dateCompactNumber |= (static_cast<uint16_t>(currentDate.day()) << 11);

    expirationDate = QString::number(dateCompactNumber, 16);
  }

  paymentMeans = pan + "f" + expirationDate + "0000";
}

void FirmwareGenerationSystem::generateFirmwareData() {}

void FirmwareGenerationSystem::assembleFirmware() {}
