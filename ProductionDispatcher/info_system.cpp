#include "info_system.h"

#include <QDate>

InfoSystem::InfoSystem(const QString& name,
                       const std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractInfoSystem{name, db} {
  loadSettings();
}

InfoSystem::~InfoSystem() {}

void InfoSystem::setContext(std::shared_ptr<ProductionLineContext> context) {
  Context = context;
}

QString InfoSystem::getTransponderBoxId(const QString& key,
                                        const QString& value) {
  SqlQueryValues transponder;

  if (!Database->readRecords(
          "transponders", QString("%1 = '%2'").arg(key, value), transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return QString();
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Транспондер c %1 = '%2' не существует.").arg(key, value));
    return QString();
  }

  return transponder.get("box_id");
}

QString InfoSystem::getTransponderPalletId(const QString& key,
                                           const QString& value) {
  SqlQueryValues transponder;
  QStringList tables{"transponders", "boxes", "pallets"};

  if (!Database->readMergedRecords(
          tables, QString("transponders.%1 = '%2'").arg(key, value),
          transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return QString();
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Транспондер c %1 = '%2' не существует.").arg(key, value));
    return QString();
  }

  return transponder.get("pallet_id");
}

ReturnStatus InfoSystem::generateTransponderData(StringDictionary& data) {
  // Данные переносимые без изменений
  data.insert("box_id", Context->box().get("id"));
  data.insert("pallet_id", Context->pallet().get("id"));
  data.insert("order_id", Context->order().get("id"));

  // Удаляем пробелы из названия модели
  QString tempModel = Context->order().get("transponder_model");
  data.insert("transponder_model", tempModel.remove(" "));

  // Конструируем серийный номер транспондера
  data.insert(
      "sn", generateTransponderSerialNumber(Context->transponder().get("id")));

  // Вычленяем символы F из personal_account_number
  QString tempPan = Context->transponder().get("personal_account_number");
  data.insert("pan", tempPan.remove(QChar('F')));

  // Название компании-заказчика
  data.insert("issuer_name", Context->order().get("name"));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateTransponderData(const QString& key,
                                                 const QString& value,
                                                 StringDictionary& data) {
  // Сохраняем контекст
  stashCurrentContext();

  loadTransponderContext(key, value);
  ReturnStatus ret = generateTransponderData(data);

  // Восстанавливаем контекст
  setContext(StashedContext);

  return ret;
}

ReturnStatus InfoSystem::generateFirmwareSeed(StringDictionary& seed) {
  // DSRC атрибуты
  seed.insert("personal_account_number",
              Context->transponder().get("personal_account_number"));
  seed.insert("id", Context->transponder().get("id"));

  seed.insert("efc_context_mark", Context->issuer().get("efc_context_mark"));

  seed.insert("manufacturer_id", Context->order().get("manufacturer_id"));
  seed.insert("equipment_class", Context->order().get("equipment_class"));
  seed.insert("transponder_model", Context->order().get("transponder_model"));
  seed.insert("accr_reference", Context->order().get("accr_reference"));

  // Генерируем дату активации батареи
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));
  seed.insert("battery_insertation_date", batteryInsertationDate.toUtf8());

  // Мастер ключи безопасности
  seed.insert("accr_key", Context->masterKeys().get("accr_key"));
  seed.insert("per_key", Context->masterKeys().get("per_key"));
  seed.insert("au_key1", Context->masterKeys().get("au_key1"));
  seed.insert("au_key2", Context->masterKeys().get("au_key2"));
  seed.insert("au_key3", Context->masterKeys().get("au_key3"));
  seed.insert("au_key4", Context->masterKeys().get("au_key4"));
  seed.insert("au_key5", Context->masterKeys().get("au_key5"));
  seed.insert("au_key6", Context->masterKeys().get("au_key6"));
  seed.insert("au_key7", Context->masterKeys().get("au_key7"));
  seed.insert("au_key8", Context->masterKeys().get("au_key8"));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateFirmwareSeed(const QString& key,
                                              const QString& value,
                                              StringDictionary& seed) {
  // Сохраняем контекст
  stashCurrentContext();

  loadTransponderContext(key, value);
  ReturnStatus ret = generateFirmwareSeed(seed);

  // Восстанавливаем контекст
  setContext(StashedContext);

  return ret;
}

ReturnStatus InfoSystem::generateBoxData(StringDictionary& data) {
  SqlQueryValues transponders;

  Database->setRecordMaxCount(0);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (!Database->readRecords(
          "transponders", QString("box_id = %1").arg(Context->box().get("id")),
          transponders)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponders.isEmpty()) {
    sendLog(QString("В боксе %1 не найдено ни одного транспондера.")
                .arg(Context->box().get("id")));
    return ReturnStatus::TranspoderMissed;
  }

  // Данные бокса
  data.insert("box_id", Context->box().get("id"));
  data.insert("box_in_process", Context->box().get("in_process"));
  data.insert("box_quantity", Context->box().get("quantity"));

  data.insert("box_assembled_units", Context->box().get("assembled_units"));
  data.insert("box_assembling_start",
              Context->box().get("assembling_start").replace("T", " "));
  data.insert("box_assembling_end",
              Context->box().get("assembling_end").replace("T", " "));

  data.insert("transponder_model", Context->order().get("transponder_model"));
  data.insert("first_transponder_sn",
              generateTransponderSerialNumber(transponders.get("id")));
  data.insert("last_transponder_sn",
              generateTransponderSerialNumber(transponders.getLast("id")));

  data.insert("pallet_id", Context->box().get("pallet_id"));
  data.insert("production_line_id", Context->box().get("production_line_id"));

  // Сохраняем модель транспондера
  QString model = Context->order().get("transponder_model");
  data.insert("transponder_model", model.remove(' '));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateBoxData(const QString& id,
                                         StringDictionary& data) {
  // Сохраняем контекст
  stashCurrentContext();

  loadBoxContext(id);
  ReturnStatus ret = generateBoxData(data);

  // Восстанавливаем контекст
  setContext(StashedContext);

  return ret;
}

ReturnStatus InfoSystem::generatePalletData(StringDictionary& data) {
  SqlQueryValues boxes;
  if (!Database->readRecords(
          "boxes",
          QString("pallet_id = %1 AND assembled_units = quantity")
              .arg(Context->box().get("id")),
          boxes)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (boxes.isEmpty()) {
    sendLog(QString("В паллете %1 не найдено ни одного собранного бокса.")
                .arg(Context->box().get("id")));
    return ReturnStatus::BoxMissed;
  }

  // Идентификатор паллеты
  data.insert("id", Context->pallet().get("id"));

  // Дата окончания сборки
  QStringList tempDate = Context->pallet().get("assembling_end").split("T");
  data.insert(
      "assembly_date",
      QDate::fromString(tempDate.first(), "yyyy-MM-dd").toString("dd.MM.yyyy"));

  // Модель транспондеров в паллете
  QString tempModel = Context->order().get("transponder_model");
  data.insert("transponder_model", tempModel.remove(" "));

  // Сохраняем идентификатор первого бокса
  data.insert("first_box_id", boxes.get("id"));

  // Сохраняем идентификатор последнего бокса
  data.insert("last_box_id", boxes.getLast("id"));

  // Общее количество транспондеров в паллете
  uint32_t totalQuantity = Context->pallet().get("assembled_units").toInt() *
                           Context->box().get("quantity").toInt();
  data.insert("quantity", QString::number(totalQuantity));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generatePalletData(const QString& id,
                                            StringDictionary& data) {
  // Сохраняем контекст
  stashCurrentContext();

  loadPalletContext(id);
  ReturnStatus ret = generatePalletData(data);

  // Восстанавливаем контекст
  setContext(StashedContext);

  return ret;
}

void InfoSystem::reset() {}

void InfoSystem::loadSettings() {}

void InfoSystem::sendLog(const QString& log) const {
  emit const_cast<InfoSystem*>(this)->logging(objectName() + " - " + log);
}

void InfoSystem::stashCurrentContext() {
  StashedContext = Context;
}

ReturnStatus InfoSystem::loadTransponderContext(const QString& key,
                                                const QString& value) {
  if (!Database->readRecords("transponders",
                             QString("%1 = '%2'").arg(key, value),
                             Context->transponder())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->transponder().isEmpty()) {
    sendLog(QString("Транспондер c %1 = '%2' не существует.").arg(key, value));
    return ReturnStatus::TranspoderMissed;
  }

  return loadBoxContext(Context->transponder().get("box_id"));
}

ReturnStatus InfoSystem::loadBoxContext(const QString& id) {
  if (!Database->readRecords(
          "boxes",
          QString("id = %1").arg(Context->productionLine().get("box_id")),
          Context->box())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->box().isEmpty()) {
    sendLog(QString("Бокс '%1' не существует.")
                .arg(Context->productionLine().get("box_id")));
    return ReturnStatus::BoxMissed;
  }

  return loadPalletContext(Context->box().get("pallet_id"));
}

ReturnStatus InfoSystem::loadPalletContext(const QString& id) {
  if (!Database->readRecords("pallets", QString("id = %1").arg(id),
                             Context->pallet())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->pallet().isEmpty()) {
    sendLog(QString("Паллета '%1' не существует.")
                .arg(Context->box().get("pallet_id")));
    return ReturnStatus::PalletMissed;
  }

  if (!Database->readRecords(
          "orders", QString("id = %1").arg(Context->pallet().get("order_id")),
          Context->order())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->order().isEmpty()) {
    sendLog(QString("Заказ '%1' не существует.")
                .arg(Context->pallet().get("order_id")));
    return ReturnStatus::OrderMissed;
  }

  if (!Database->readRecords(
          "issuers", QString("id = %1").arg(Context->order().get("issuer_id")),
          Context->issuer())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->issuer().isEmpty()) {
    sendLog(QString("Заказчик '%1' не существует.")
                .arg(Context->order().get("issuer_id")));
    return ReturnStatus::IssuerMissed;
  }

  // В зависимости от типа персонализации, берем те или иные мастер ключи
  QString masterKeyTable;
  QString masterKeyFK;
  if (Context->order().get("full_personalization") == "true") {
    masterKeyTable = "commercial_master_keys";
    masterKeyFK = "commercial_master_keys_id";
  } else {
    masterKeyTable = "transport_master_keys";
    masterKeyFK = "transport_master_keys_id";
  }
  if (!Database->readRecords(
          masterKeyTable,
          QString("id = %2").arg(Context->issuer().get(masterKeyFK)),
          Context->masterKeys())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->masterKeys().isEmpty()) {
    sendLog(QString("Группа мастер ключей '%1' не существует.")
                .arg(Context->issuer().get(masterKeyFK)));
    return ReturnStatus::MasterKeysMissed;
  }

  return ReturnStatus::NoError;
}

void InfoSystem::initContext(void) {
  Context = std::shared_ptr<ProductionLineContext>(new ProductionLineContext());
}

QString InfoSystem::generateTransponderSerialNumber(const QString& id) const {
  // Преобразуем в десятичный формат
  QString manufacturerId = QString::number(
      Context->order().get("manufacturer_id").toInt(nullptr, 16));

  // Дата сборки
  QStringList tempDate = Context->box().get("assembling_start").split("T");
  QDate date = QDate::fromString(tempDate.first(), "yyyy-MM-dd");
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));

  // Дополняем серийник до 10 цифр нулями слева
  QString extendedTransponderId = QString("%1").arg(id, 10, QChar('0'));

  // Конструируем серийный номер транспондера
  return QString("%1%2%3").arg(manufacturerId, batteryInsertationDate,
                               extendedTransponderId);
}
