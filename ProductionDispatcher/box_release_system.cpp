#include "box_release_system.h"
#include "definitions.h"

BoxReleaseSystem::BoxReleaseSystem(const QString& name,
                                   std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractBoxReleaseSystem(name, db) {
  loadSettings();
}

BoxReleaseSystem::~BoxReleaseSystem() {}

void BoxReleaseSystem::setContext(
    std::shared_ptr<ProductionLineContext> context) {
  Context = context;
}

ReturnStatus BoxReleaseSystem::request() {
  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }
  sendLog("Запрос.");

  if (!Context->box().isEmpty() ||
      (Context->productionLine().get("box_id") != "0") ||
      (Context->productionLine().get("transponder_id") != "0")) {
    sendLog(QString("Производственная линия %1 уже имеет бокс для сборки.")
                .arg(Context->login()));
    return ReturnStatus::BoxAlreadyRequested;
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

  if (!attachBox()) {
    return ReturnStatus::DatabaseQueryError;
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

ReturnStatus BoxReleaseSystem::refund() {
  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }
  sendLog("Возврат.");

  if (Context->box().isEmpty() &&
      (Context->productionLine().get("box_id") == "0") &&
      (Context->productionLine().get("transponder_id") == "0")) {
    sendLog(
        QString(
            "Производственная линия '%1' не связана ни с каким боксом. Возврат "
            "не требуется.")
            .arg(Context->login()));
    return ReturnStatus::BoxNotRequested;
  }
  sendLog(QString("Осуществление возврата бокса %1.")
              .arg(Context->box().get("id")));

  if (!stopBoxAssembly()) {
    return ReturnStatus::DatabaseQueryError;
  }

  SqlQueryValues boxesInProcess;
  if (!Database->readRecords("boxes",
                             QString("pallet_id = %1 AND in_process = true")
                                 .arg(Context->pallet().get("id")),
                             boxesInProcess)) {
    sendLog(QString("Получена ошибка при получении данных из таблицы boxes."));
    return ReturnStatus::DatabaseQueryError;
  }

  // Если в паллете нет боксов, находящиеся в процессе сборки, то
  // останавливаем сборку паллеты
  if (boxesInProcess.isEmpty()) {
    if (!stopPalletAssembly()) {
      return ReturnStatus::DatabaseQueryError;
    }
  }

  // Отвязываем производственную линию от бокса
  if (!detachBox()) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus BoxReleaseSystem::complete() {
  ReturnStatus ret = ReturnStatus::NoError;

  if (!Context->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }
  sendLog("Завершить сборку.");

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

  // Проверка на переполнение паллеты
  if (Context->pallet().get("assembled_units") >=
      Context->pallet().get("quantity")) {
    sendLog(
        QString("Палета %1 переполнена. Завершить сборку бокса %1 невозможно.")
            .arg(Context->pallet().get("id"), Context->box().get("id")));
    return ReturnStatus::PalletOverflow;
  }

  // Завершаем процесс сборки бокса
  newBox.add("in_process", "false");
  newBox.add("completed", "true");
  newBox.add("assembling_end", QDateTime::currentDateTime().toString(
                                   POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateBox(newBox)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Увеличиваем счетчик выпущенных боксов в паллете
  newPallet.add(
      "assembled_units",
      QString::number(Context->pallet().get("assembled_units").toInt() + 1));
  if (!updatePallet(newPallet)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Если паллета целиком собрана
  if (Context->pallet().get("assembled_units").toInt() ==
      Context->pallet().get("quantity").toInt()) {
    // Завершаем сборку паллеты
    ret = completePallet();
    if (ret != ReturnStatus::NoError) {
      return ret;
    }
  }

  // Если заказ целиком собран
  if (Context->order().get("assembled_units").toInt() ==
      Context->order().get("quantity").toInt()) {
    // Завершаем сборку заказа
    ret = completeOrder();
    if (ret != ReturnStatus::NoError) {
      return ret;
    }
  }

  // Отвязываем производственную линию от бокса
  if (!detachBox()) {
    return ReturnStatus::DatabaseQueryError;
  }

  // В противном случае возвращаемся
  return ReturnStatus::NoError;
}

void BoxReleaseSystem::clearContext() {
  sendLog(QString("Очистка контекста бокса %1.").arg(Context->box().get("id")));

  Context->box().clear();
  Context->transponder().clear();
  Context->pallet().clear();
}

void BoxReleaseSystem::loadSettings() {}

void BoxReleaseSystem::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
}

ReturnStatus BoxReleaseSystem::findOrderInProcess() {
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

ReturnStatus BoxReleaseSystem::findBox() {
  QStringList tables{"boxes", "pallets", "orders"};
  Database->setRecordMaxCount(1);
  Database->setRecordMaxCount(Qt::AscendingOrder);

  if (Context->order().get("assembled_units") ==
      Context->order().get("quantity")) {
    sendLog(
        QString("Заказ %1 полностью собран.").arg(Context->order().get("id")));
    return ReturnStatus::OrderCompletelyAssembled;
  }

  if (!Database->readMergedRecords(
          tables,
          QString("boxes.production_line_id = %1 AND orders.id = %2 AND "
                  "boxes.completed = false AND boxes.assembled_units <= "
                  "boxes.quantity AND boxes.completed = false")
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
                  "boxes.assembled_units = 0 AND boxes.completed = false")
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

bool BoxReleaseSystem::attachBox() {
  SqlQueryValues plNew;

  plNew.add("box_id", Context->box().get("id"));
  plNew.add("in_process", "true");
  if (!updateProductionLine(plNew)) {
    sendLog(QString("Не удалось связать производственную линию '%1' с "
                    "боксом %2.")
                .arg(Context->login(), Context->box().get("id")));
    return false;
  }

  sendLog(QString("Производственная линия '%1' успешно связана с "
                  "боксом %2.")
              .arg(Context->login(), Context->box().get("id")));
  return true;
}

bool BoxReleaseSystem::detachBox() {
  SqlQueryValues plNew;

  plNew.add("transponder_id", "NULL");
  plNew.add("box_id", "NULL");
  plNew.add("transponder_id", "NULL");
  plNew.add("in_process", "false");
  if (!updateProductionLine(plNew)) {
    sendLog(QString("Не удалось отвязать производственную линию '%1' от "
                    "бокса %2.")
                .arg(Context->login(), Context->box().get("id")));
    return false;
  }

  sendLog(QString("Производственная линия '%1' успешно отвязана от "
                  "бокса %2.")
              .arg(Context->login(), Context->box().get("id")));
  return true;
}

ReturnStatus BoxReleaseSystem::startBoxAssembly() {
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

ReturnStatus BoxReleaseSystem::startPalletAssembly() {
  SqlQueryValues palletNew;

  palletNew.add("assembling_start", QDateTime::currentDateTime().toString(
                                        POSTGRES_TIMESTAMP_TEMPLATE));
  palletNew.add("in_process", "true");
  if (!updatePallet(palletNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

bool BoxReleaseSystem::stopBoxAssembly() {
  SqlQueryValues boxNew;

  if (Context->box().get("assembled_units") == "0") {
    boxNew.add("production_line_id", "NULL");
    boxNew.add("assembling_start", "NULL");
  }
  boxNew.add("in_process", "false");
  boxNew.add("id", Context->box().get("id"));
  if (!updateBox(boxNew)) {
    sendLog(QString("Не удалось остановить процесс сборки бокса %1.")
                .arg(Context->box().get("id")));
    return false;
  }

  sendLog(QString("Процесс сборки бокса %1 остановлен.")
              .arg(Context->box().get("id")));
  return true;
}

bool BoxReleaseSystem::stopPalletAssembly() {
  SqlQueryValues palletNew;

  palletNew.add("id", Context->pallet().get("id"));
  palletNew.add("in_process", "false");
  if (Context->pallet().get("assembled_units") == "0") {
    palletNew.add("assembling_start", "NULL");
  }
  if (!updatePallet(palletNew)) {
    sendLog(QString("Не удалось остановить процесс сборки паллеты %1.")
                .arg(Context->pallet().get("id")));
    return false;
  }

  sendLog(QString("Процесс сборки паллеты %1 остановлен.")
              .arg(Context->pallet().get("id")));
  return true;
}

ReturnStatus BoxReleaseSystem::completePallet() {
  SqlQueryValues newPallet;
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки паллеты
  newPallet.add("in_process", "false");
  newPallet.add("assembling_end", QDateTime::currentDateTime().toString(
                                      POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updatePallet(newPallet)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Увеличиваем счетчик выпущенных паллет в заказе
  newOrder.add(
      "assembled_units",
      QString::number(Context->order().get("assembled_units").toInt() + 1));
  if (!updateOrder(newOrder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // В противном случае возвращаемся
  return ReturnStatus::NoError;
}

ReturnStatus BoxReleaseSystem::completeOrder() {
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки заказа
  newOrder.add("in_process", "false");
  newOrder.add("assembling_end", QDateTime::currentDateTime().toString(
                                     POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateOrder(newOrder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus BoxReleaseSystem::loadBoxContext() {
  Database->setCurrentOrder(Qt::AscendingOrder);
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
    return ReturnStatus::PalletMissed;
  }

  return ReturnStatus::NoError;
}

bool BoxReleaseSystem::updateProductionLine(const SqlQueryValues& newValues) {
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

bool BoxReleaseSystem::updateTransponder(const SqlQueryValues& newValues) {
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

bool BoxReleaseSystem::updateBox(const SqlQueryValues& newValues) {
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

bool BoxReleaseSystem::updatePallet(const SqlQueryValues& newValues) {
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

bool BoxReleaseSystem::updateOrder(const SqlQueryValues& newValues) {
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
