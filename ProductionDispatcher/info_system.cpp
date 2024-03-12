#include "info_system.h"

#include <QDate>

InfoSystem::InfoSystem(const QString& name) : AbstractInfoSystem{name} {}

InfoSystem::~InfoSystem() {}

ReturnStatus InfoSystem::updateMainContext() {
  MainContext->clear();

  Database->setRecordMaxCount(0);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (!Database->readRecords("orders", QString("in_process = true"),
                             MainContext->order())) {
    sendLog(QString("Получена ошибка при получении данных из таблицы order."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->order().isEmpty()) {
    sendLog(QString(
        "Получена ошибка при поиске заказа, находящегося в процессе сборки."));
    return ReturnStatus::OrderInProcessMissed;
  }

  if (MainContext->order().recordCount() > 1) {
    sendLog(
        QString("В системе одновременно находится более одного заказа в "
                "процессе сборки."));
    return ReturnStatus::OrderMultiplyAssembly;
  }

  if (!Database->readRecords(
          "issuers",
          QString("id = '%1'").arg(MainContext->order().get("issuer_id")),
          MainContext->issuer())) {
    sendLog(
        QString("Получена ошибка при получении данных из таблицы issuers."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->issuer().isEmpty()) {
    sendLog(QString("Получена ошибка при поиске эмитента, которому принадлежит "
                    "заказ '%1'.")
                .arg(MainContext->order().get("id")));
    return ReturnStatus::IssuerMissed;
  }

  QString keyTable;
  QString keyTableRef;
  if (MainContext->order().get("full_personalization") == "false") {
    keyTable = "transport_master_keys";
    keyTableRef = "transport_master_keys_id";
  } else {
    keyTable = "commercial_master_keys";
    keyTableRef = "commercial_master_keys_id";
  }

  if (!Database->readRecords(
          keyTable,
          QString("id = '%1'").arg(MainContext->issuer().get(keyTableRef)),
          MainContext->masterKeys())) {
    sendLog(QString("Получена ошибка при получении данных из таблицы %1.")
                .arg(keyTable));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->masterKeys().isEmpty()) {
    sendLog(QString("Получена ошибка при поиске мастер ключей для заказа %1.")
                .arg(MainContext->order().get("id")));
    return ReturnStatus::MasterKeysMissed;
  }

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateProductionLineData(StringDictionary& data) {
  if (!SubContext->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не запущена.")
                .arg(SubContext->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }
  sendLog("Генерация данных производственной линии.");

  // Данные переносимые без изменений
  data.insert("production_line_id", SubContext->productionLine().get("id"));
  data.insert("production_line_login",
              SubContext->productionLine().get("login"));
  data.insert("production_line_ns",
              QString("%1 %2").arg(SubContext->productionLine().get("surname"),
                                   SubContext->productionLine().get("name")));
  data.insert("production_line_in_process",
              SubContext->productionLine().get("in_process"));
  data.insert("transponder_id",
              SubContext->productionLine().get("transponder_id"));
  data.insert("box_id", SubContext->productionLine().get("box_id"));

  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::AscendingOrder);
  SqlQueryValues response;
  if (!Database->execCustomRequest(
          QString("SELECT COUNT(*) FROM boxes WHERE DATE(assembling_end) = "
                  "CURRENT_DATE AND production_line_id = %1")
              .arg(SubContext->productionLine().get("id")),
          response)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }
  data.insert("today_assembled_boxes", response.get(0));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateTransponderData(StringDictionary& data) {
  if (SubContext->transponder().isEmpty()) {
    sendLog(
        QString(
            "Производственная линия %1 не связана ни с каким транспондером.")
            .arg(SubContext->login()));
    return ReturnStatus::TransponderMissed;
  }
  sendLog("Генерация данных транспондера.");

  // Данные переносимые без изменений
  data.insert("box_id", SubContext->box().get("id"));
  data.insert("transponder_release_counter",
              SubContext->transponder().get("release_counter"));
  data.insert("transponder_ucid", SubContext->transponder().get("ucid"));

  // Конструируем серийный номер транспондера
  data.insert("transponder_sn", generateTransponderSerialNumber(
                                    SubContext->transponder().get("id")));

  // Вычленяем символы F из personal_account_number
  QString tempPan = SubContext->transponder().get("personal_account_number");
  data.insert("transponder_pan", tempPan.remove(QChar('F')));

  // Название компании-заказчика
  data.insert("issuer_name", MainContext->issuer().get("name"));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateTransponderData(const QString& key,
                                                 const QString& value,
                                                 StringDictionary& data) {
  // Сохраняем контекст
  saveContexts();

  ReturnStatus ret = loadTransponderContext(key, value);
  if (ret != ReturnStatus::NoError) {
    sendLog(
        QString("Получена ошибка при загрузке контекста транспондера с %1=%2.")
            .arg(key, value));
    return ret;
  }
  ret = generateTransponderData(data);

  // Восстанавливаем контекст
  restoreSavedContexts();

  if (ret != ReturnStatus::NoError) {
    sendLog(
        QString("Получена ошибка при генерации данных транспондера с %1=%2.")
            .arg(key, value));
    return ret;
  }

  return ret;
}

ReturnStatus InfoSystem::generateFirmwareSeed(StringDictionary& seed) {
  if (SubContext->transponder().isEmpty()) {
    sendLog(
        QString(
            "Производственная линия %1 не связана ни с каким транспондером.")
            .arg(SubContext->login()));
    return ReturnStatus::TransponderMissed;
  }
  sendLog("Генерация сида прошивки.");

  // DSRC атрибуты
  seed.insert("personal_account_number",
              SubContext->transponder().get("personal_account_number"));
  seed.insert("id", SubContext->transponder().get("id"));

  seed.insert("efc_context_mark",
              MainContext->issuer().get("efc_context_mark"));

  seed.insert("manufacturer_id", MainContext->order().get("manufacturer_id"));
  seed.insert("equipment_class", MainContext->order().get("equipment_class"));
  seed.insert("transponder_model",
              MainContext->order().get("transponder_model"));
  seed.insert("accr_reference", MainContext->order().get("accr_reference"));

  // Генерируем дату активации батареи
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));
  seed.insert("battery_insertation_date", batteryInsertationDate.toUtf8());

  // Мастер ключи безопасности
  seed.insert("accr_key", MainContext->masterKeys().get("accr_key"));
  seed.insert("per_key", MainContext->masterKeys().get("per_key"));
  seed.insert("au_key1", MainContext->masterKeys().get("au_key1"));
  seed.insert("au_key2", MainContext->masterKeys().get("au_key2"));
  seed.insert("au_key3", MainContext->masterKeys().get("au_key3"));
  seed.insert("au_key4", MainContext->masterKeys().get("au_key4"));
  seed.insert("au_key5", MainContext->masterKeys().get("au_key5"));
  seed.insert("au_key6", MainContext->masterKeys().get("au_key6"));
  seed.insert("au_key7", MainContext->masterKeys().get("au_key7"));
  seed.insert("au_key8", MainContext->masterKeys().get("au_key8"));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateFirmwareSeed(const QString& key,
                                              const QString& value,
                                              StringDictionary& seed) {
  // Сохраняем контекст
  saveContexts();

  ReturnStatus ret = loadTransponderContext(key, value);
  if (ret != ReturnStatus::NoError) {
    sendLog(
        QString("Получена ошибка при загрузке контекста транспондера с %1=%2.")
            .arg(key, value));
    return ret;
  }
  ret = generateFirmwareSeed(seed);

  // Восстанавливаем контекст
  restoreSavedContexts();

  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при генерации сида прошивки для "
                    "транспондера с %1=%2.")
                .arg(key, value));
    return ret;
  }

  return ret;
}

ReturnStatus InfoSystem::generateBoxData(StringDictionary& data) {
  if (SubContext->box().isEmpty()) {
    sendLog(QString("Производственная линия %1 не связана ни с каким боксом.")
                .arg(SubContext->login()));
    return ReturnStatus::BoxMissed;
  }
  sendLog("Генерация данных бокса.");

  Database->setRecordMaxCount(0);
  Database->setCurrentOrder(Qt::AscendingOrder);

  SqlQueryValues transponders;
  if (!Database->readRecords(
          "transponders",
          QString("box_id = %1").arg(SubContext->box().get("id")),
          transponders)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponders.isEmpty()) {
    sendLog(QString("В боксе %1 не найдено ни одного транспондера.")
                .arg(SubContext->box().get("id")));
    return ReturnStatus::BoxIsEmty;
  }

  // Данные бокса
  data.insert("box_id", SubContext->box().get("id"));
  data.insert("box_in_process", SubContext->box().get("in_process"));
  data.insert("box_quantity", SubContext->box().get("quantity"));

  data.insert("box_assembled_units", SubContext->box().get("assembled_units"));
  data.insert("box_assembling_start",
              SubContext->box().get("assembling_start").replace("T", " "));
  data.insert("box_assembling_end",
              SubContext->box().get("assembling_end").replace("T", " "));

  data.insert("transponder_model",
              MainContext->order().get("transponder_model"));
  data.insert("first_transponder_sn",
              generateTransponderSerialNumber(transponders.get("id")));
  data.insert("last_transponder_sn",
              generateTransponderSerialNumber(transponders.getLast("id")));

  data.insert("pallet_id", SubContext->box().get("pallet_id"));
  data.insert("production_unit", SubContext->productionLine().get("login"));

  // Сохраняем модель транспондера
  QString model = MainContext->order().get("transponder_model");
  data.insert("transponder_model", model.remove(' '));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::generateBoxData(const QString& id,
                                         StringDictionary& data) {
  // Сохраняем контекст
  saveContexts();

  ReturnStatus ret = loadBoxContext(id);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при загрузке контекста бокса с id=%1.")
                .arg(id));
    return ret;
  }
  ret = generateBoxData(data);

  // Восстанавливаем контекст
  restoreSavedContexts();

  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при генерации данных паллеты с id=%1.")
                .arg(id));
    return ret;
  }

  return ret;
}

ReturnStatus InfoSystem::generatePalletData(StringDictionary& data) {
  QString palletId = SubContext->box().get("pallet_id");

  if (MainContext->pallet(palletId).isEmpty()) {
    sendLog(QString("Производственная линия %1 не связана ни с какой паллетой.")
                .arg(SubContext->login()));
    return ReturnStatus::PalletMissed;
  }
  sendLog("Генерация данных паллеты.");

  return generatePalletDataSubprocess(palletId, data);
}

ReturnStatus InfoSystem::generatePalletData(const QString& id,
                                            StringDictionary& data) {
  // Сохраняем контекст
  saveContexts();

  ReturnStatus ret = loadPalletContext(id);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при загрузке контекста паллеты с id=%1.")
                .arg(id));
    return ret;
  }
  ret = generatePalletDataSubprocess(id, data);

  // Восстанавливаем контекст
  restoreSavedContexts();

  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при генерации данных паллеты с id=%1.")
                .arg(id));
    return ret;
  }

  return ret;
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

  transponder.addField("pallet_id");
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

void InfoSystem::saveContexts() {
  SavedMainContext = MainContext;
  SavedSubContext = SubContext;

  MainContext = std::shared_ptr<ProductionContext>(new ProductionContext());
  SubContext =
      std::shared_ptr<ProductionLineContext>(new ProductionLineContext());
}

void InfoSystem::restoreSavedContexts() {
  MainContext = SavedMainContext;
  SubContext = SavedSubContext;
}

ReturnStatus InfoSystem::generatePalletDataSubprocess(const QString& id,
                                                      StringDictionary& data) {
  SqlQueryValues boxes;
  if (!Database->readRecords("boxes", QString("pallet_id = %1").arg(id),
                             boxes)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (boxes.isEmpty()) {
    sendLog(QString("В паллете %1 не найдено ни одного бокса.").arg(id));
    return ReturnStatus::PalletIsEmpty;
  }

  // Идентификатор паллеты
  data.insert("pallet_id", id);

  // Дата окончания сборки
  QStringList tempDate =
      MainContext->pallet(id).get("assembling_end").split("T");
  data.insert(
      "pallet_assembly_date",
      QDate::fromString(tempDate.first(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
  if (data["pallet_assembly_date"] == "") {
    data["pallet_assembly_date"] = "NaN";
  }

  // Модель транспондеров в паллете
  QString tempModel = MainContext->order().get("transponder_model");
  data.insert("transponder_model", tempModel.remove(" "));

  // Сохраняем идентификатор первого бокса
  data.insert("first_box_id", boxes.get("id"));

  // Сохраняем идентификатор последнего бокса
  data.insert("last_box_id", boxes.getLast("id"));

  // Общее количество транспондеров в паллете
  uint32_t totalQuantity =
      MainContext->pallet(id).get("assembled_units").toInt() *
      boxes.get("quantity").toInt();
  data.insert("pallet_quantity", QString::number(totalQuantity));

  return ReturnStatus::NoError;
}

ReturnStatus InfoSystem::loadTransponderContext(const QString& key,
                                                const QString& value) {
  if (!Database->readRecords("transponders",
                             QString("%1 = '%2'").arg(key, value),
                             SubContext->transponder())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (SubContext->transponder().isEmpty()) {
    sendLog(QString("Транспондер c %1 = '%2' не существует.").arg(key, value));
    return ReturnStatus::TransponderMissed;
  }

  return loadBoxContext(SubContext->transponder().get("box_id"));
}

ReturnStatus InfoSystem::loadBoxContext(const QString& id) {
  if (!Database->readRecords("boxes", QString("id = %1").arg(id),
                             SubContext->box())) {
    sendLog(QString("Получена ошибка при поиске данных бокса %1.").arg(id));
    return ReturnStatus::DatabaseQueryError;
  }

  if (SubContext->box().isEmpty()) {
    sendLog(QString("Бокс '%1' не существует.").arg(id));
    return ReturnStatus::BoxMissed;
  }

  QString plId = SubContext->box().get("production_line_id");

  if (!Database->readRecords("production_lines", QString("id = %1").arg(plId),
                             SubContext->productionLine())) {
    sendLog(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'.")
            .arg(plId));
    return ReturnStatus::DatabaseQueryError;
  }

  if (SubContext->productionLine().isEmpty()) {
    sendLog(QString("Производственная линия '%1' не существует.").arg(plId));
    return ReturnStatus::BoxMissed;
  }

  return loadPalletContext(SubContext->box().get("pallet_id"));
}

ReturnStatus InfoSystem::loadPalletContext(const QString& id) {
  if (!Database->readRecords("pallets", QString("id = %1").arg(id),
                             MainContext->pallet(id))) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->pallet(id).isEmpty()) {
    sendLog(QString("Паллета '%1' не существует.")
                .arg(SubContext->box().get("pallet_id")));
    return ReturnStatus::PalletMissed;
  }

  if (!Database->readRecords(
          "orders",
          QString("id = %1").arg(MainContext->pallet(id).get("order_id")),
          MainContext->order())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->order().isEmpty()) {
    sendLog(QString("Заказ '%1' не существует.")
                .arg(MainContext->pallet(id).get("order_id")));
    return ReturnStatus::OrderMissed;
  }

  if (!Database->readRecords(
          "issuers",
          QString("id = %1").arg(MainContext->order().get("issuer_id")),
          MainContext->issuer())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->issuer().isEmpty()) {
    sendLog(QString("Заказчик '%1' не существует.")
                .arg(MainContext->order().get("issuer_id")));
    return ReturnStatus::IssuerMissed;
  }

  // В зависимости от типа персонализации, берем те или иные мастер ключи
  QString masterKeyTable;
  QString masterKeyFK;
  if (MainContext->order().get("full_personalization") == "true") {
    masterKeyTable = "commercial_master_keys";
    masterKeyFK = "commercial_master_keys_id";
  } else {
    masterKeyTable = "transport_master_keys";
    masterKeyFK = "transport_master_keys_id";
  }
  if (!Database->readRecords(
          masterKeyTable,
          QString("id = %2").arg(MainContext->issuer().get(masterKeyFK)),
          MainContext->masterKeys())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->masterKeys().isEmpty()) {
    sendLog(QString("Группа мастер ключей '%1' не существует.")
                .arg(MainContext->issuer().get(masterKeyFK)));
    return ReturnStatus::MasterKeysMissed;
  }

  return ReturnStatus::NoError;
}

QString InfoSystem::generateTransponderSerialNumber(const QString& id) const {
  // Преобразуем в десятичный формат
  QString manufacturerId = QString::number(
      MainContext->order().get("manufacturer_id").toInt(nullptr, 16));

  // Дата сборки
  QStringList tempDate = SubContext->box().get("assembling_start").split("T");
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
