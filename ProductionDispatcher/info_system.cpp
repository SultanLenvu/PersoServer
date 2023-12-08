#include "info_system.h"
#include "Log/log_system.h"

InfoSystem::InfoSystem(const QString& name,
                       const std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractInfoSystem{name, db} {
  loadSettings();

  ContextReady = false;
}

InfoSystem::~InfoSystem() {}

void InfoSystem::setContext(const ProductionContext& context) {
  CurrentProductionLine = context.value("production_line");
  CurrentTransponder = context.value("transponder");
  CurrentBox = context.value("box");
  CurrentPallet = context.value("pallet");
  CurrentOrder = context.value("order");
  CurrentIssuer = context.value("issuer");
  CurrentMasterKeys = context.value("master_keys");

  ContextReady = true;
}

ReturnStatus InfoSystem::generateProductionContext(const QString& login,
                                                   ProductionContext& context) {
  // Подготовка
  initContext(context);

  // Заполнение
  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (!Database->readRecords("production_lines",
                             QString("login = '%1'").arg("login"),
                             *CurrentProductionLine)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentProductionLine->isEmpty()) {
    sendLog(QString("Производственной линии '%1' не существует.").arg(login));
    return ReturnStatus::ProductionLineMissed;
  }

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

  if (!Database->readRecords(
          "transponders",
          QString("id = %1 AND release_counter = 0")
              .arg(CurrentProductionLine->get("transponder_id")),
          *CurrentTransponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (CurrentTransponder->isEmpty()) {
    sendLog(QString("Транспондер '%1' не существует.")
                .arg(CurrentProductionLine->get("transponder_id")));
    return ReturnStatus::TranspoderMissed;
  }

  if (!Database->readRecords(
          "pallets", QString("id = %1").arg(CurrentBox->get("pallet_id")),
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

ReturnStatus InfoSystem::generateTransponderData(StringDictionary& data) {
  if (!ContextReady) {
    return ReturnStatus::ContextError;
  }

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

ReturnStatus InfoSystem::generateFirmwareSeed(StringDictionary& seed) {
  if (!ContextReady) {
    return ReturnStatus::ContextError;
  }

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

ReturnStatus InfoSystem::generateBoxData(StringDictionary& data) {
  if (!ContextReady) {
    return ReturnStatus::ContextError;
  }

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

ReturnStatus InfoSystem::generatePalletData(StringDictionary& data) {
  if (!ContextReady) {
    return ReturnStatus::ContextError;
  }

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

void InfoSystem::initContext(ProductionContext& context) {
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
