#include "info_system.h"
#include "Log/log_system.h"

InfoSystem::InfoSystem(const QString& name,
                       const std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractInfoSystem{name, db} {
  loadSettings();
}

InfoSystem::~InfoSystem() {}

void InfoSystem::setContext(const ProductionLineContext& context) {
  CurrentProductionLine = context.value("production_line");
  CurrentTransponder = context.value("transponder");
  CurrentBox = context.value("box");
  CurrentPallet = context.value("pallet");
  CurrentOrder = context.value("order");
  CurrentIssuer = context.value("issuer");
  CurrentMasterKeys = context.value("master_keys");
}

ReturnStatus InfoSystem::loadProductionLineContext(
    const QString& login,
    ProductionLineContext& context) {
  // Подготовка
  initContext(context);
  setContext(context);

  // Заполнение
  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (!Database->readRecords("production_lines",
                             QString("%1 = '%2'").arg("login", login),
                             *CurrentProductionLine)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentProductionLine->isEmpty()) {
    sendLog(QString("Производственной линии c %1 = %2 не существует.")
                .arg("login", login));
    return ReturnStatus::ProductionLineMissed;
  }

  return loadTransponderContext("id",
                                CurrentProductionLine->get("transponder_id"));
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
  data.insert("box_id", CurrentBox->get("id"));
  data.insert("pallet_id", CurrentPallet->get("id"));
  data.insert("order_id", CurrentOrder->get("id"));

  // Удаляем пробелы из названия модели
  QString tempModel = CurrentOrder->get("transponder_model");
  data.insert("transponder_model", tempModel.remove(" "));

  // Конструируем серийный номер транспондера
  data.insert("sn",
              generateTransponderSerialNumber(CurrentTransponder->get("id")));

  // Вычленяем символы F из personal_account_number
  QString tempPan = CurrentTransponder->get("personal_account_number");
  data.insert("pan", tempPan.remove(QChar('F')));

  // Название компании-заказчика
  data.insert("issuer_name", CurrentIssuer->get("name"));

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
              CurrentTransponder->get("personal_account_number"));
  seed.insert("id", CurrentTransponder->get("id"));

  seed.insert("efc_context_mark", CurrentIssuer->get("efc_context_mark"));

  seed.insert("manufacturer_id", CurrentOrder->get("manufacturer_id"));
  seed.insert("equipment_class", CurrentOrder->get("equipment_class"));
  seed.insert("transponder_model", CurrentOrder->get("transponder_model"));
  seed.insert("accr_reference", CurrentOrder->get("accr_reference"));

  // Генерируем дату активации батареи
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));
  seed.insert("battery_insertation_date", batteryInsertationDate.toUtf8());

  // Мастер ключи безопасности
  seed.insert("accr_key", CurrentMasterKeys->get("accr_key"));
  seed.insert("per_key", CurrentMasterKeys->get("per_key"));
  seed.insert("au_key1", CurrentMasterKeys->get("au_key1"));
  seed.insert("au_key2", CurrentMasterKeys->get("au_key2"));
  seed.insert("au_key3", CurrentMasterKeys->get("au_key3"));
  seed.insert("au_key4", CurrentMasterKeys->get("au_key4"));
  seed.insert("au_key5", CurrentMasterKeys->get("au_key5"));
  seed.insert("au_key6", CurrentMasterKeys->get("au_key6"));
  seed.insert("au_key7", CurrentMasterKeys->get("au_key7"));
  seed.insert("au_key8", CurrentMasterKeys->get("au_key8"));

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
  if (!Database->readRecords("transponders",
                             QString("box_id = %1 AND release_counter > 0")
                                 .arg(CurrentBox->get("id")),
                             transponders)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponders.isEmpty()) {
    sendLog(QString("В боксе %1 не найдено ни одного транспондера.")
                .arg(CurrentBox->get("id")));
    return ReturnStatus::TranspoderMissed;
  }

  // Идентификатор бокса
  data.insert("id", CurrentBox->get("id"));

  // Количество транспондеров в боксе
  data.insert("quantity", CurrentBox->get("assembled_units"));

  // Сохраняем серийник первого транспондера в боксе
  data.insert("first_transponder_sn",
              generateTransponderSerialNumber(transponders.get("id")));

  // Сохраняем серийник последнего транспондера в боксе
  data.insert("last_transponder_sn",
              generateTransponderSerialNumber(transponders.getLast("id")));

  // Сохраняем модель транспондера
  QString model = CurrentOrder->get("transponder_model");
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
              .arg(CurrentBox->get("id")),
          boxes)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (boxes.isEmpty()) {
    sendLog(QString("В паллете %1 не найдено ни одного собранного бокса.")
                .arg(CurrentBox->get("id")));
    return ReturnStatus::BoxMissed;
  }

  // Идентификатор паллеты
  data.insert("id", CurrentPallet->get("id"));

  // Дата окончания сборки
  QStringList tempDate = CurrentPallet->get("assembling_end").split("T");
  data.insert(
      "assembly_date",
      QDate::fromString(tempDate.first(), "yyyy-MM-dd").toString("dd.MM.yyyy"));

  // Модель транспондеров в паллете
  QString tempModel = CurrentOrder->get("transponder_model");
  data.insert("transponder_model", tempModel.remove(" "));

  // Сохраняем идентификатор первого бокса
  data.insert("first_box_id", boxes.get("id"));

  // Сохраняем идентификатор последнего бокса
  data.insert("last_box_id", boxes.getLast("id"));

  // Общее количество транспондеров в паллете
  uint32_t totalQuantity = CurrentPallet->get("assembled_units").toInt() *
                           CurrentBox->get("quantity").toInt();
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

void InfoSystem::reset() {
  CurrentProductionLine.reset();
  CurrentTransponder.reset();
  CurrentBox.reset();
  CurrentPallet.reset();
  CurrentOrder.reset();
  CurrentIssuer.reset();
  CurrentMasterKeys.reset();
}

void InfoSystem::loadSettings() {}

void InfoSystem::sendLog(const QString& log) const {
  LogSystem::instance()->generate(objectName() + " - " + log);
}

void InfoSystem::stashCurrentContext() {
  StashedContext.clear();
  StashedContext.insert("production_line", CurrentProductionLine);
  StashedContext.insert("transponder", CurrentTransponder);
  StashedContext.insert("box", CurrentBox);
  StashedContext.insert("pallet", CurrentPallet);
  StashedContext.insert("order", CurrentOrder);
  StashedContext.insert("issuer", CurrentIssuer);
  StashedContext.insert("master_keys", CurrentMasterKeys);
}

ReturnStatus InfoSystem::loadTransponderContext(const QString& key,
                                                const QString& value) {
  if (!Database->readRecords("transponders",
                             QString("%1 = '%2'").arg(key, value),
                             *CurrentTransponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentTransponder->isEmpty()) {
    sendLog(QString("Транспондер c %1 = '%2' не существует.").arg(key, value));
    return ReturnStatus::TranspoderMissed;
  }

  return loadBoxContext(CurrentTransponder->get("box_id"));
}

ReturnStatus InfoSystem::loadBoxContext(const QString& id) {
  if (!Database->readRecords(
          "boxes", QString("id = %1").arg(CurrentProductionLine->get("box_id")),
          *CurrentBox)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentBox->isEmpty()) {
    sendLog(QString("Бокс '%1' не существует.")
                .arg(CurrentProductionLine->get("box_id")));
    return ReturnStatus::BoxMissed;
  }

  return loadPalletContext(CurrentBox->get("pallet_id"));
}

ReturnStatus InfoSystem::loadPalletContext(const QString& id) {
  if (!Database->readRecords("pallets", QString("id = %1").arg(id),
                             *CurrentPallet)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentPallet->isEmpty()) {
    sendLog(QString("Паллета '%1' не существует.")
                .arg(CurrentBox->get("pallet_id")));
    return ReturnStatus::PalletMissed;
  }

  if (!Database->readRecords(
          "orders", QString("id = %1").arg(CurrentPallet->get("order_id")),
          *CurrentOrder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentOrder->isEmpty()) {
    sendLog(QString("Заказ '%1' не существует.")
                .arg(CurrentPallet->get("order_id")));
    return ReturnStatus::OrderMissed;
  }

  if (!Database->readRecords(
          "issuers", QString("id = %1").arg(CurrentOrder->get("issuer_id")),
          *CurrentIssuer)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentIssuer->isEmpty()) {
    sendLog(QString("Заказчик '%1' не существует.")
                .arg(CurrentOrder->get("issuer_id")));
    return ReturnStatus::IssuerMissed;
  }

  // В зависимости от типа персонализации, берем те или иные мастер ключи
  QString masterKeyTable;
  QString masterKeyFK;
  if (CurrentOrder->get("full_personalization") == "true") {
    masterKeyTable = "commercial_master_keys";
    masterKeyFK = "commercial_master_keys_id";
  } else {
    masterKeyTable = "transport_master_keys";
    masterKeyFK = "transport_master_keys_id";
  }
  if (!Database->readRecords(
          masterKeyTable,
          QString("id = %2").arg(CurrentIssuer->get(masterKeyFK)),
          *CurrentMasterKeys)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentMasterKeys->isEmpty()) {
    sendLog(QString("Группа мастер ключей '%1' не существует.")
                .arg(CurrentIssuer->get(masterKeyFK)));
    return ReturnStatus::MasterKeysMissed;
  }

  return ReturnStatus::NoError;
}

void InfoSystem::initContext(ProductionLineContext& context) {
  context.clear();
  context.insert("production_line",
                 std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
  context.insert("transponder",
                 std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
  context.insert("box", std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
  context.insert("pallet",
                 std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
  context.insert("order",
                 std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
  context.insert("issuer",
                 std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
  context.insert("master_keys",
                 std::shared_ptr<SqlQueryValues>(new SqlQueryValues()));
}

QString InfoSystem::generateTransponderSerialNumber(const QString& id) const {
  // Преобразуем в десятичный формат
  QString manufacturerId =
      QString::number(CurrentOrder->get("manufacturer_id").toInt(nullptr, 16));

  // Дата сборки
  QStringList tempDate = CurrentBox->get("assembling_start").split("T");
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
