#include <QDateTime>

#include "production_line_launch_system.h"
#include "Database/sql_query_values.h"
#include "General/definitions.h"

ProductionLineLaunchSystem::ProductionLineLaunchSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractLaunchSystem(name, db) {
  loadSettings();
}

ProductionLineLaunchSystem::~ProductionLineLaunchSystem() {}

void ProductionLineLaunchSystem::setContext(
    std::shared_ptr<ProductionLineContext> context) {
  Context = context;
}

ReturnStatus ProductionLineLaunchSystem::init() {
  SqlQueryValues newValues;

  newValues.add("launched", "false");
  newValues.add("in_process", "false");
  newValues.add("box_id", "NULL");
  newValues.add("transponder_id", "NULL");
  if (!Database->updateRecords("production_lines", newValues)) {
    sendLog(QString(
        "Получена ошибка при обновлении данных производственных линии. "));
    return ReturnStatus::DatabaseQueryError;
  }

  sendLog("Инициализация производственных линий успешно завершена.");
  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::launch() {
  ReturnStatus ret;

  ret = loadProductionLine();
  if (ret != ReturnStatus::NoError) {
    return ret;
  }
  ret = checkProductionLineState();
  if (ret != ReturnStatus::NoError) {
    return ret;
  }
  sendLog(QString("Данные производственной линии загружены. "));

  ret = loadOrderInProcess();
  if (ret != ReturnStatus::NoError) {
    return ret;
  }
  sendLog(QString("Данные заказа находящегося в процессе сборки загружены. "));

  SqlQueryValues newValues;
  newValues.add("launched", "true");
  if (!updateProductionLine(newValues)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::shutdown() {
  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена. Остановка "
                    "не требуется.")
                .arg(Context->login()));
    return ReturnStatus::NoError;
  }

  SqlQueryValues plNew;
  Database->setRecordMaxCount(1);

  plNew.add("launched", "false");
  if (!updateProductionLine(plNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  ReturnStatus ret = detachFromBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось отвязать производственную линию '%1' от "
                    "бокса %1.")
                .arg(Context->login(), Context->box().get("id")));
    return ret;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::requestBox() {
  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }

  ReturnStatus ret;

  ret = findBox();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось найти бокс.");
    return ret;
  }

  ret = loadBoxContext();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось загрузить контекст бокса.");
    return ret;
  }

  ret = attachWithBox();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось связать производственную линию с боксом.");
    return ret;
  }

  ret = startBoxAssembly();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось загрузить запустить сборку бокса.");
    return ReturnStatus::DatabaseQueryError;
  }

  // Если паллета не в процессе сборки
  if (Context->pallet().get("in_process") == "false") {
    ret = startPalletAssembly();
    if (ret != ReturnStatus::NoError) {
      sendLog("Не удалось загрузить запустить сборку паллеты.");
      return ReturnStatus::DatabaseQueryError;
    }
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::refundBox() {
  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }

  ReturnStatus ret = detachFromBox();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось отвязать производственную линию '%1' от "
                    "бокса %1.")
                .arg(Context->login(), Context->box().get("id")));
    return ret;
  }

  ret = stopBoxAssembly();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось остановить процесс сборки бокса %1.")
                .arg(Context->box().get("id")));
    return ret;
  }

  SqlQueryValues boxesInProcess;
  if (!Database->readRecords("boxes",
                             QString("pallet_id = %1 AND in_process = true")
                                 .arg(Context->pallet().get("id")),
                             boxesInProcess)) {
    sendLog(QString("Получена ошибка при получении данных из таблицы boxes."));
    return ReturnStatus::DatabaseQueryError;
  }

  // Если в паллете все еще есть боксы, находящиеся в процессе сборки, то
  // возвращаемся
  if (!boxesInProcess.isEmpty()) {
    return ReturnStatus::NoError;
  }
  // В противном случае останавливаем сборку паллеты
  ret = stopPalletAssembly();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось остановить процесс сборки паллеты %1.")
                .arg(Context->pallet().get("id")));
    return ret;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::completeBox() {
  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }

  SqlQueryValues newBox;
  SqlQueryValues newPallet;

  // Проверка возможности завершения сборки бокса
  if (Context->box().get("assembled_units") != Context->box().get("quantity")) {
    sendLog(QString("Не удалось завершить сборку бокса %1, поскольку не все "
                    "транспондеры в "
                    "нем были собраны.")
                .arg(Context->box().get("id")));
    return ReturnStatus::BoxNotCompletelyAssembled;
  }

  // Завершаем процесс сборки бокса
  newBox.add("in_process", "false");
  newBox.add("assembling_end", QDateTime::currentDateTime().toString(
                                   POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateBox(newBox)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Отправляем сигнал о завершении сборки бокса
  //  std::shared_ptr<QString> boxId(new QString(Context->box().value("id")));
  //  emit boxAssemblyCompleted(boxId);

  // Увеличиваем счетчик выпущенных боксов в паллете
  newPallet.add(
      "assembled_units",
      QString::number(Context->pallet().get("assembled_units").toInt() + 1));
  newPallet.add("assembling_end", "NULL");
  if (!updatePallet(newPallet)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Если паллета целиком собрана
  if (Context->pallet().get("assembled_units").toInt() ==
      Context->pallet().get("quantity").toInt()) {
    // Завершаем сборку паллеты
    return completePallet();
  }

  // Если заказ целиком собран
  if (Context->order().get("assembled_units").toInt() ==
      Context->order().get("quantity").toInt()) {
    // Завершаем сборку заказа
    return completeOrder();
  }

  // В противном случае возвращаемся
  return ReturnStatus::NoError;
}

void ProductionLineLaunchSystem::loadSettings() {}

void ProductionLineLaunchSystem::sendLog(const QString& log) {
  emit logging(objectName() + " - " + log);
}

ReturnStatus ProductionLineLaunchSystem::checkProductionLineState() {
  if (Context->productionLine().get("active") == "false") {
    sendLog(
        QString(
            "Производственная линия '%1' не активирована. Запуск невозможен.")
            .arg(Context->login()));
    return ReturnStatus::ProductionLineNotActive;
  }

  if (Context->productionLine().get("launched") == "true") {
    sendLog(QString("Производственная линия '%1' уже запущена. Повторный "
                    "запуск невозможен.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineAlreadyLaunched;
  }

  if (Context->productionLine().get("in_process") == "true") {
    sendLog(QString("Производственная линия '%1' уже находится в процессе "
                    "работы. Повторный "
                    "запуск невозможен.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineAlreadyInProcess;
  }

  if (Context->productionLine().get("completed") == "true") {
    sendLog(QString("Производственная линия '%1' завершила свою работу. Запуск "
                    "невозможен.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineCompleted;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::findOrderInProcess() {
  Database->setRecordMaxCount(0);

  if (!Database->readRecords("orders", QString("in_process = true"),
                             Context->order())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->order().recordCount() > 1) {
    sendLog(
        QString("В системе присутствует более одного заказа, находящихся в "
                "процессе сборки."));
    return ReturnStatus::OrderMultiplyAssembly;
  }

  if (Context->order().recordCount() == 0) {
    sendLog(
        QString("В системе отсутствуют заказ, находящийся в процессе сборки."));
    return ReturnStatus::OrderInProcessMissed;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::findBox() {
  QStringList tables{"boxes", "pallets", "orders"};
  Database->setRecordMaxCount(1);
  Database->setRecordMaxCount(Qt::AscendingOrder);

  Context->box().clear();

  if (!Database->readMergedRecords(
          tables,
          QString("boxes.production_line_id = %1 AND orders.id = %2 AND "
                  "boxes.assembled_units < boxes.quantity")
              .arg(Context->productionLine().get("id"),
                   Context->order().get("id")),
          Context->box())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (!Context->box().isEmpty()) {
    sendLog(QString("В заказе %1 у производственной линии '%2' найден "
                    "незавершенный бокс %3.")
                .arg(Context->order().get("id"), Context->login(),
                     Context->box().get("id")));
    return ReturnStatus::NoError;
  }
  sendLog(QString("В заказе %1 у производственной линии '%1' нет "
                  "незавершенных боксов.")
              .arg(Context->order().get("id"), Context->login()));

  sendLog(QString("Поиск свободного бокса в заказе %1.")
              .arg(Context->order().get("id")));
  if (!Database->readMergedRecords(
          tables,
          QString("boxes.production_line_id IS NULL AND orders.id = %1 AND "
                  "boxes.assembled_units = 0")
              .arg(Context->order().get("id")),
          Context->box())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->box().isEmpty()) {
    sendLog(QString("Получена ошибка при поиске свободного бокса в заказе %1.")
                .arg(Context->order().get("id")));
    return ReturnStatus::FreeBoxMissed;
  }

  sendLog(QString("Свободный бокс найден, идентификатор: %1.")
              .arg(Context->box().get("id")));
  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::attachWithBox() {
  SqlQueryValues plNew;

  plNew.add("box_id", Context->box().get("id"));
  plNew.add("transponder_id", Context->transponder().get("id"));
  plNew.add("in_process", "true");
  if (!updateProductionLine(plNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::detachFromBox() {
  SqlQueryValues plNew;

  plNew.add("transponder_id", "NULL");
  plNew.add("box_id", "NULL");
  plNew.add("transponder_id", "NULL");
  plNew.add("in_process", "false");
  if (!updateProductionLine(plNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::startBoxAssembly() {
  SqlQueryValues boxNew;

  if (Context->box().get("assembled_units") == "0") {
    boxNew.add("production_line_id", Context->productionLine().get("id"));
    boxNew.add("assembling_start", QDateTime::currentDateTime().toString(
                                       POSTGRES_TIMESTAMP_TEMPLATE));
  }
  boxNew.add("in_process", "true");
  if (!updateBox(boxNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::startPalletAssembly() {
  SqlQueryValues palletNew;

  palletNew.add("assembling_start", QDateTime::currentDateTime().toString(
                                        POSTGRES_TIMESTAMP_TEMPLATE));
  palletNew.add("in_process", "true");
  if (!updatePallet(palletNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::stopBoxAssembly() {
  SqlQueryValues boxNew;

  if (Context->box().get("assembled_units") == "0") {
    boxNew.add("production_line_id", "NULL");
    boxNew.add("assembling_start", "NULL");
  }
  boxNew.add("in_process", "false");
  boxNew.add("id", Context->box().get("id"));
  if (!updateBox(boxNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::stopPalletAssembly() {
  SqlQueryValues palletNew;

  palletNew.add("in_process", "false");
  palletNew.add("id", Context->box().get("id"));
  if (!updatePallet(palletNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::completePallet() {
  SqlQueryValues newPallet;
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки паллеты
  newPallet.add("in_process", "false");
  newPallet.add("assembling_end", QDateTime::currentDateTime().toString(
                                      POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updatePallet(newPallet)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Отправляем сигнал о завершении сборки паллеты
  //  std::shared_ptr<QString> palletId(new
  //  QString(Context->pallet().get("id"))); emit
  //  palletAssemblyCompleted(palletId);

  // Увеличиваем счетчик выпущенных паллет в заказе
  newOrder.add(
      "assembled_units",
      QString::number(Context->order().get("assembled_units").toInt() + 1));
  newOrder.add("assembling_end", "NULL");
  if (!updateOrder(newOrder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // В противном случае возвращаемся
  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::completeOrder() {
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки заказа
  newOrder.add("in_process", "false");
  newOrder.add("assembling_end", QDateTime::currentDateTime().toString(
                                     POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateOrder(newOrder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Отправляем сигнал о завершении сборки заказа
  //  std::shared_ptr<QString> orderId(new QString(Context->order().get("id")));
  //  emit palletAssemblyCompleted(orderId);

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::loadBoxContext() {
  // Если все транспондеры в боксе были собраны,
  // то останавливаемся на последнем и ожидаем заверешения сборки бокса

  Database->setCurrentOrder(Qt::AscendingOrder);

  if (Context->box().get("assembled_units") == Context->box().get("quantity")) {
    // Дополнительная проверка консистентности системы
    int32_t quant = Context->box().get("quantity").toInt();

    Database->setRecordMaxCount(quant);

    SqlQueryValues transponders;
    if (!Database->readRecords("transponders",
                               QString("box_id = '%1' AND release_counter > 0")
                                   .arg(Context->box().get("id")),
                               transponders)) {
      sendLog(QString(
          "Получена ошибка при получении данных из таблицы transponders."));
      return ReturnStatus::DatabaseQueryError;
    }

    if (transponders.recordCount() != quant) {
      sendLog(
          QString("Количество выпущенных транспондеров в боксе не "
                  "соответствует счетчику."));
      return ReturnStatus::ConsistencyViolation;
    }

    Database->setRecordMaxCount(1);
    Context->transponder().addRecord(transponders.getRecord(quant - 1));
  } else {  // В противном случае ищем первый невыпущенный транспондер в боксе
    Database->setRecordMaxCount(1);
    if (!Database->readRecords("transponders",
                               QString("release_counter = 0 AND box_id = '%1'")
                                   .arg(Context->box().get("id")),
                               Context->transponder())) {
      sendLog(QString(
          "Получена ошибка при получении данных из таблицы transponders."));
      return ReturnStatus::DatabaseQueryError;
    }
  }

  if (Context->transponder().isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске очередного транспондера в боксе %1.")
            .arg(Context->box().get("id")));
    return ReturnStatus::FreeBoxMissed;
  }

  Database->setRecordMaxCount(1);

  if (!Database->readRecords(
          "pallets", QString("id = '%1'").arg(Context->box().get("pallet_id")),
          Context->pallet())) {
    sendLog(
        QString("Получена ошибка при получении данных из таблицы pallets."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->pallet().isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске паллеты, в которой содержится бокс %1.")
            .arg(Context->box().get("id")));
    return ReturnStatus::FreeBoxMissed;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::loadProductionLine() {
  Context->productionLine().clear();
  Database->setRecordMaxCount(1);

  if (!Database->readRecords("production_lines",
                             QString("login = '%1' AND password = '%2'")
                                 .arg(Context->login(), Context->password()),
                             Context->productionLine())) {
    sendLog(QString(
        "Получена ошибка при получении данных из таблицы production_lines."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->productionLine().isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'.")
            .arg(Context->login()));
    return ReturnStatus::ProductionLineMissed;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::loadOrderInProcess() {
  Context->order().clear();
  Context->issuer().clear();
  Context->masterKeys().clear();
  Database->setRecordMaxCount(0);

  if (!Database->readRecords("orders", QString("in_process = true"),
                             Context->order())) {
    sendLog(QString("Получена ошибка при получении данных из таблицы order."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->order().isEmpty()) {
    sendLog(QString(
        "Получена ошибка при поиске  заказа, находящегося в процессе сборки."));
    return ReturnStatus::OrderInProcessMissed;
  }

  if (Context->order().recordCount() > 1) {
    sendLog(
        QString("В системе одновременно находится более одного заказа в "
                "процессе сборки."));
    return ReturnStatus::OrderMultiplyAssembly;
  }

  if (!Database->readRecords(
          "issuers",
          QString("id = '%1'").arg(Context->order().get("issuer_id")),
          Context->issuer())) {
    sendLog(
        QString("Получена ошибка при получении данных из таблицы issuers."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->issuer().isEmpty()) {
    sendLog(QString("Получена ошибка при поиске эмитента, которому принадлежит "
                    "заказ '%1'.")
                .arg(Context->order().get("id")));
    return ReturnStatus::IssuerMissed;
  }

  QString keyTable;
  QString keyTableRef;
  if (Context->order().get("full_personalization") == "false") {
    keyTable = "transport_master_keys";
    keyTableRef = "transport_master_keys_id";
  } else {
    keyTable = "commercial_master_keys";
    keyTableRef = "commercial_master_keys_id";
  }

  if (!Database->readRecords(
          keyTable,
          QString("id = '%1'").arg(Context->issuer().get(keyTableRef)),
          Context->masterKeys())) {
    sendLog(QString("Получена ошибка при получении данных из таблицы %1.")
                .arg(keyTable));
    return ReturnStatus::DatabaseQueryError;
  }

  if (Context->masterKeys().isEmpty()) {
    sendLog(QString("Получена ошибка при поиске мастер ключей для заказа %1.")
                .arg(Context->order().get("id")));
    return ReturnStatus::MasterKeysMissed;
  }

  return ReturnStatus::NoError;
}

bool ProductionLineLaunchSystem::updateProductionLine(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "production_lines",
          QString("id = '%1'").arg(Context->productionLine().get("id")),
          newValues)) {
    sendLog(
        QString(
            "Получена ошибка при обновлении данных производственной линии %1. ")
            .arg(Context->productionLine().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "production_lines",
          QString("id = %1").arg(Context->productionLine().get("id")),
          Context->productionLine())) {
    sendLog(
        QString(
            "Получена ошибка при получении данных производственной линии %1. ")
            .arg(Context->productionLine().get("id")));
    return false;
  }

  return true;
}

bool ProductionLineLaunchSystem::updateTransponder(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "transponders",
          QString("id = %1").arg(Context->transponder().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных транспондера %1. ")
                .arg(Context->transponder().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "transponders",
          QString("id = %1").arg(Context->transponder().get("id")),
          Context->transponder())) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(Context->transponder().get("id")));
    return false;
  }

  return true;
}

bool ProductionLineLaunchSystem::updateBox(const SqlQueryValues& newValues) {
  if (!Database->updateRecords("boxes",
                               QString("id = %1").arg(Context->box().get("id")),
                               newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных бокса %1. ")
                .arg(Context->box().get("id")));
    return false;
  }

  if (!Database->readRecords("boxes",
                             QString("id = %1").arg(Context->box().get("id")),
                             Context->box())) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ")
                .arg(Context->box().get("id")));
    return false;
  }

  return true;
}

bool ProductionLineLaunchSystem::updatePallet(const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "pallets", QString("id = %1").arg(Context->pallet().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных паллеты %1. ")
                .arg(Context->pallet().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "pallets", QString("id = %1").arg(Context->pallet().get("id")),
          Context->pallet())) {
    sendLog(QString("Получена ошибка при получении данных паллеты %1. ")
                .arg(Context->pallet().get("id")));
    return false;
  }

  return true;
}

bool ProductionLineLaunchSystem::updateOrder(const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "orders", QString("id = %1").arg(Context->order().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных заказа %1. ")
                .arg(Context->order().get("id")));
    return false;
  }

  if (!Database->readRecords("orders",
                             QString("id = %1").arg(Context->order().get("id")),
                             Context->order())) {
    sendLog(QString("Получена ошибка при получении данных заказа %1. ")
                .arg(Context->order().get("id")));
    return false;
  }

  return true;
}
