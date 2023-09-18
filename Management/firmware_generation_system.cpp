#include "firmware_generation_system.h"

FirmwareGenerationSystem::FirmwareGenerationSystem(QObject *parent) : QObject(parent)
{
  setObjectName("FirmwareGenerationSystem");

  FirmwareBase = new QFile(this);
  FirmwareData = new QFile(this);

  // Загружаем настройки
  loadSettings();

  // Создаем карту позиционирования данных в прошивке
  createPositionMap();
}

void FirmwareGenerationSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
}

bool FirmwareGenerationSystem::generate(TransponderSeedModel* seed,
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

  firmware->replace(
      EFC_CONTEXT_MARK_FPI, EFC_CONTEXT_MARK_SIZE,
      QByteArray::fromHex(
          seed->attributes()->value("efc_context_mark").toUtf8()));

  firmware->replace(
      EFC_CONTEXT_MARK_FPI, EFC_CONTEXT_MARK_SIZE,
      QByteArray::fromHex(
          seed->attributes()->value("efc_context_mark").toUtf8()));

  //  attributes->insert("transponder_model", "");
  //  attributes->insert("release_counter", "");
  //  attributes->insert("awaiting_confirmation", "");
  //  attributes->insert("ucid", "");
  //  attributes->insert("accr_reference", "");
  //  attributes->insert("payment_means", "");
  //  attributes->insert("efc_context_mark", "");

  //  masterKeys->insert("accr_key", "");
  //  masterKeys->insert("per_key", "");
  //  masterKeys->insert("au_key1", "");
  //  masterKeys->insert("au_key2", "");
  //  masterKeys->insert("au_key3", "");
  //  masterKeys->insert("au_key4", "");
  //  masterKeys->insert("au_key5", "");
  //  masterKeys->insert("au_key6", "");
  //  masterKeys->insert("au_key7", "");
  //  masterKeys->insert("au_key8", "");

  FirmwareData->close();
  return true;
}

void FirmwareGenerationSystem::loadSettings() {
  QSettings settings;

  FirmwareBase->setFileName(settings.value("Firmware/Base/Path").toString());
  FirmwareData->setFileName(settings.value("Firmware/Data/Path").toString());

  // Сделать дополнение файлов до
}

void FirmwareGenerationSystem::createPositionMap() {
  PositionMap.insert("ManufacturerId", 0);
  PositionMap.insert("ManufacturingSerialNo", 0);
  PositionMap.insert("EquipmentClass", 0);
  PositionMap.insert("TransponderPartNo", 0);
  PositionMap.insert("AccrReference", 0);
  PositionMap.insert("BatteryInsertationDate", 0);

  PositionMap.insert("EfcContextMark", 0);
  PositionMap.insert("EquipmentObuId", 0);
  PositionMap.insert("PaymentMeans", 0);

  PositionMap.insert("AccrKey", 0);
  PositionMap.insert("PerKey", 0);
  PositionMap.insert("AuKey1", 0);
  PositionMap.insert("AuKey2", 0);
  PositionMap.insert("AuKey3", 0);
  PositionMap.insert("AuKey4", 0);
  PositionMap.insert("AuKey5", 0);
  PositionMap.insert("AuKey6", 0);
  PositionMap.insert("AuKey7", 0);
  PositionMap.insert("AuKey8", 0);
}

void FirmwareGenerationSystem::generateCommonKeys(TransponderSeedModel* seed) {}

void FirmwareGenerationSystem::generateFirmwareData() {}

void FirmwareGenerationSystem::assembleFirmware() {}
