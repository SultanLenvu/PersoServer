#include "box_release_system.h"
#include "definitions.h"

BoxReleaseSystem::BoxReleaseSystem(const QString& name)
    : AbstractBoxReleaseSystem(name) {}

BoxReleaseSystem::~BoxReleaseSystem() {}

ReturnStatus BoxReleaseSystem::request() {
  ReturnStatus ret;

  if (!SubContext->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена.")
                .arg(SubContext->login()));
    return ReturnStatus::ProductionLineNotLaunched;
  }
  sendLog("Запрос.");

  if (!SubContext->box().isEmpty()) {
    sendLog(QString("Производственная линия %1 уже связана с боксом %2.")
                .arg(SubContext->login(), SubContext->box().get("id")));
    return ReturnStatus::BoxAlreadyRequested;
  }

  ret = findBox();
  if (ret != ReturnStatus::NoError) {
    sendLog("Не удалось найти бокс.");
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

  QString palletId = SubContext->box().get("pallet_id");
  ret = loadPalletData(palletId);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось загрузить запустить данные паллеты %1.")
                .arg(palletId));
    return ret;
  }

  if (MainContext->pallet(palletId).get("in_process") == "false") {
    ret = startPalletAssembly(palletId);
    if (ret != ReturnStatus::NoError) {
      sendLog(QString("Не удалось загрузить запустить сборку паллеты %1.")
                  .arg(palletId));
      return ReturnStatus::DatabaseQueryError;
    }
  }

  return ReturnStatus::NoError;
}

ReturnStatus BoxReleaseSystem::refund() {
  if (!SubContext->isInProcess()) {
    sendLog(
        QString("Производственная линия '%1' не находится в процессе сборки.")
            .arg(SubContext->login()));
    return ReturnStatus::ProductionLineNotInProcess;
  }
  sendLog("Возврат.");

  sendLog(QString("Осуществление возврата бокса %1.")
              .arg(SubContext->box().get("id")));

  if (!stopBoxAssembly()) {
    return ReturnStatus::DatabaseQueryError;
  }

  QString palletId = SubContext->box().get("pallet_id");
  // Проверка на существование паллеты
  if (MainContext->pallet(palletId).isEmpty()) {
    sendLog(QString("Паллета %1 не опреределена в производственном контесте. ")
                .arg(palletId));
    return ReturnStatus::PalletMissed;
  }

  SqlQueryValues boxesInProcess;
  if (!Database->readRecords(
          "boxes",
          QString("pallet_id = %1 AND in_process = true").arg(palletId),
          boxesInProcess)) {
    sendLog(QString("Получена ошибка при получении данных из таблицы boxes."));
    return ReturnStatus::DatabaseQueryError;
  }

  // Если в паллете нет боксов, находящиеся в процессе сборки, то
  // останавливаем сборку паллеты
  if (boxesInProcess.isEmpty()) {
    if (!stopPalletAssembly(palletId)) {
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
  SqlQueryValues newBox;
  SqlQueryValues newPallet;

  if (!SubContext->isInProcess()) {
    sendLog(
        QString("Производственная линия '%1' не находится в процессе сборки.")
            .arg(SubContext->login()));
    return ReturnStatus::ProductionLineNotInProcess;
  }
  sendLog("Завершить сборку.");

  // Проверка возможности завершения сборки бокса
  if (SubContext->box().get("assembled_units") !=
      SubContext->box().get("quantity")) {
    sendLog(QString("Не удалось завершить сборку бокса %1, поскольку не все "
                    "транспондеры в нем были собраны.")
                .arg(SubContext->box().get("id")));
    return ReturnStatus::BoxNotCompletelyAssembled;
  }

  // Завершаем процесс сборки бокса
  newBox.add("in_process", "false");
  newBox.add("completed", "true");
  newBox.add("assembling_end", QDateTime::currentDateTime().toString(
                                   POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateBox(newBox)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Оправляем сигнал о завершении сборки бокса
  emit boxAssemblyCompleted(ret);
  if (ret != ReturnStatus::NoError) {
    return ret;
  }

  // Проверка на существование паллеты
  QString palletId = SubContext->box().get("pallet_id");
  if (MainContext->pallet(palletId).isEmpty()) {
    sendLog(QString("Паллета %1 не опреределена в производственном контесте. ")
                .arg(palletId));
    return ReturnStatus::PalletMissed;
  }

  // Проверка на переполнение паллеты
  if (MainContext->pallet(palletId).get("assembled_units").toInt() >=
      MainContext->pallet(palletId).get("quantity").toInt()) {
    sendLog(
        QString("Палета %1 переполнена. Завершить сборку бокса %2 невозможно.")
            .arg(palletId, SubContext->box().get("id")));
    return ReturnStatus::PalletOverflow;
  }

  // Увеличиваем счетчик выпущенных боксов в паллете
  newPallet.add(
      "assembled_units",
      QString::number(
          MainContext->pallet(palletId).get("assembled_units").toInt() + 1));
  if (!updatePallet(palletId, newPallet)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Если паллета целиком собрана
  if (MainContext->pallet(palletId).get("assembled_units").toInt() ==
      MainContext->pallet(palletId).get("quantity").toInt()) {
    // Завершаем сборку паллеты
    ret = completePallet(palletId);
    if (ret != ReturnStatus::NoError) {
      return ret;
    }
  }

  // Если заказ целиком собран
  if (MainContext->order().get("assembled_units").toInt() ==
      MainContext->order().get("quantity").toInt()) {
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

ReturnStatus BoxReleaseSystem::findBox() {
  QStringList tables{"boxes", "pallets", "orders"};
  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (MainContext->order().get("assembled_units") ==
      MainContext->order().get("quantity")) {
    sendLog(QString("Заказ %1 полностью собран.")
                .arg(MainContext->order().get("id")));
    return ReturnStatus::OrderCompletelyAssembled;
  }

  if (!Database->readMergedRecords(
          tables,
          QString("boxes.production_line_id = %1 AND orders.id = %2 AND "
                  "boxes.completed = false AND boxes.assembled_units <= "
                  "boxes.quantity AND boxes.completed = false")
              .arg(SubContext->productionLine().get("id"),
                   MainContext->order().get("id")),
          SubContext->box())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (!SubContext->box().isEmpty()) {
    sendLog(QString("В заказе %1 у производственной линии '%2' найден "
                    "незавершенный бокс %3.")
                .arg(MainContext->order().get("id"), SubContext->login(),
                     SubContext->box().get("id")));
    return ReturnStatus::NoError;
  }
  sendLog(QString("В заказе %1 у производственной линии '%2' нет "
                  "незавершенных боксов.")
              .arg(MainContext->order().get("id"), SubContext->login()));

  sendLog(QString("Поиск свободного бокса в заказе %1.")
              .arg(MainContext->order().get("id")));
  if (!Database->readMergedRecords(
          tables,
          QString("boxes.production_line_id IS NULL AND orders.id = %1 AND "
                  "boxes.assembled_units = 0 AND boxes.completed = false")
              .arg(MainContext->order().get("id")),
          SubContext->box())) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (SubContext->box().isEmpty()) {
    //    SqlQueryValues boxInProcess;
    //    if (!Database->readMergedRecords(
    //            tables,
    //            QString("orders.id = %1 AND boxes.in_process = true")
    //                .arg(MainContext->order().get("id")),
    //            boxInProcess)) {
    //      sendLog(QString("Получена ошибка при выполнении запроса в базу
    //      данных.")); return ReturnStatus::DatabaseQueryError;
    //    }
    //    // Если в заказе еще есть боксы в процессе сборки -> заказ собран
    //    if (!boxInProcess.isEmpty()) {
    //      sendLog(QString("Заказ %1 полностью собран.")
    //                  .arg(MainContext->order().get("id")));
    //      return ReturnStatus::OrderCompletelyAssembled;
    //    }
    // В противном случае в заказе нет свободных боксов
    sendLog(QString("Получена ошибка при поиске свободного бокса в заказе %1.")
                .arg(MainContext->order().get("id")));
    return ReturnStatus::FreeBoxMissed;
  }

  sendLog(QString("Свободный бокс найден, идентификатор: %1.")
              .arg(SubContext->box().get("id")));
  return ReturnStatus::NoError;
}

bool BoxReleaseSystem::attachBox() {
  SqlQueryValues plNew;

  plNew.add("box_id", SubContext->box().get("id"));
  plNew.add("in_process", "true");
  if (!updateProductionLine(plNew)) {
    sendLog(QString("Не удалось связать производственную линию '%1' с "
                    "боксом %2.")
                .arg(SubContext->login(), SubContext->box().get("id")));
    return false;
  }

  sendLog(QString("Производственная линия '%1' успешно связана с "
                  "боксом %2.")
              .arg(SubContext->login(), SubContext->box().get("id")));
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
                .arg(SubContext->login(), SubContext->box().get("id")));
    return false;
  }

  SubContext->box().clear();
  SubContext->transponder().clear();

  sendLog(QString("Производственная линия '%1' успешно отвязана от "
                  "бокса %2.")
              .arg(SubContext->login(), SubContext->box().get("id")));
  return true;
}

ReturnStatus BoxReleaseSystem::startBoxAssembly() {
  SqlQueryValues boxNew;

  if (SubContext->box().get("assembled_units") == "0") {
    boxNew.add("production_line_id", SubContext->productionLine().get("id"));
    boxNew.add("assembling_start", QDateTime::currentDateTime().toString(
                                       POSTGRES_TIMESTAMP_TEMPLATE));
  }
  boxNew.add("in_process", "true");
  if (!updateBox(boxNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus BoxReleaseSystem::startPalletAssembly(const QString& id) {
  SqlQueryValues palletNew;

  palletNew.add("assembling_start", QDateTime::currentDateTime().toString(
                                        POSTGRES_TIMESTAMP_TEMPLATE));
  palletNew.add("in_process", "true");
  if (!updatePallet(id, palletNew)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

bool BoxReleaseSystem::stopBoxAssembly() {
  SqlQueryValues boxNew;

  if (SubContext->box().get("assembled_units") == "0") {
    boxNew.add("production_line_id", "NULL");
    boxNew.add("assembling_start", "NULL");
  }
  boxNew.add("in_process", "false");
  boxNew.add("id", SubContext->box().get("id"));
  if (!updateBox(boxNew)) {
    sendLog(QString("Не удалось остановить процесс сборки бокса %1.")
                .arg(SubContext->box().get("id")));
    return false;
  }

  sendLog(QString("Процесс сборки бокса %1 остановлен.")
              .arg(SubContext->box().get("id")));
  return true;
}

bool BoxReleaseSystem::stopPalletAssembly(const QString& id) {
  SqlQueryValues palletNew;

  palletNew.add("in_process", "false");
  if (MainContext->pallet(id).get("assembled_units") == "0") {
    palletNew.add("assembling_start", "NULL");
  }
  if (!updatePallet(id, palletNew)) {
    sendLog(
        QString("Не удалось остановить процесс сборки паллеты %1.").arg(id));
    return false;
  }

  MainContext->removePallet(id);

  sendLog(QString("Процесс сборки паллеты %1 остановлен.").arg(id));
  return true;
}

ReturnStatus BoxReleaseSystem::completePallet(const QString& id) {
  ReturnStatus ret = ReturnStatus::NoError;
  SqlQueryValues newPallet;
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки паллеты
  newPallet.add("in_process", "false");
  newPallet.add("assembling_end", QDateTime::currentDateTime().toString(
                                      POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updatePallet(id, newPallet)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Увеличиваем счетчик выпущенных паллет в заказе
  newOrder.add(
      "assembled_units",
      QString::number(MainContext->order().get("assembled_units").toInt() + 1));
  if (!updateOrder(newOrder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Отправляем сигнал о завершении сборки паллетыS
  emit palletAssemblyCompleted(ret);
  if (ret != ReturnStatus::NoError) {
    return ret;
  }

  sendLog(QString("Процесс сборки паллеты %1 завершен.").arg(id));
  return ReturnStatus::NoError;
}

ReturnStatus BoxReleaseSystem::completeOrder() {
  ReturnStatus ret = ReturnStatus::NoError;
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки заказа
  newOrder.add("in_process", "false");
  newOrder.add("assembling_end", QDateTime::currentDateTime().toString(
                                     POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateOrder(newOrder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Отправляем сигнал о завершении сборки заказа
  emit orderAssemblyCompleted(ret);
  if (ret != ReturnStatus::NoError) {
    return ret;
  }

  return ret;
}

ReturnStatus BoxReleaseSystem::loadPalletData(const QString& id) {
  Database->setCurrentOrder(Qt::AscendingOrder);
  Database->setRecordMaxCount(1);

  if (!Database->readRecords(
          "pallets",
          QString("id = '%1'").arg(SubContext->box().get("pallet_id")),
          MainContext->pallet(id))) {
    sendLog(
        QString("Получена ошибка при получении данных из таблицы pallets."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (MainContext->pallet(id).isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске паллеты, в которой содержится бокс %1.")
            .arg(SubContext->box().get("id")));
    return ReturnStatus::PalletMissed;
  }

  return ReturnStatus::NoError;
}

bool BoxReleaseSystem::updateProductionLine(const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "production_lines",
          QString("id = '%1'").arg(SubContext->productionLine().get("id")),
          newValues)) {
    sendLog(
        QString(
            "Получена ошибка при обновлении данных производственной линии %1. ")
            .arg(SubContext->productionLine().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "production_lines",
          QString("id = %1").arg(SubContext->productionLine().get("id")),
          SubContext->productionLine())) {
    sendLog(
        QString(
            "Получена ошибка при получении данных производственной линии %1. ")
            .arg(SubContext->productionLine().get("id")));
    return false;
  }

  return true;
}

bool BoxReleaseSystem::updateTransponder(const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "transponders",
          QString("id = %1").arg(SubContext->transponder().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных транспондера %1. ")
                .arg(SubContext->transponder().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "transponders",
          QString("id = %1").arg(SubContext->transponder().get("id")),
          SubContext->transponder())) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(SubContext->transponder().get("id")));
    return false;
  }

  return true;
}

bool BoxReleaseSystem::updateBox(const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "boxes", QString("id = %1").arg(SubContext->box().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных бокса %1. ")
                .arg(SubContext->box().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "boxes", QString("id = %1").arg(SubContext->box().get("id")),
          SubContext->box())) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ")
                .arg(SubContext->box().get("id")));
    return false;
  }

  return true;
}

bool BoxReleaseSystem::updatePallet(const QString& id,
                                    const SqlQueryValues& newValues) {
  if (!Database->updateRecords("pallets", QString("id = %1").arg(id),
                               newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных паллеты %1. ")
                .arg(SubContext->box().get("id")));
    return false;
  }

  if (!Database->readRecords("pallets", QString("id = %1").arg(id),
                             MainContext->pallet(id))) {
    sendLog(
        QString("Получена ошибка при получении данных паллеты %1. ").arg(id));
    return false;
  }

  return true;
}

bool BoxReleaseSystem::updateOrder(const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "orders", QString("id = %1").arg(MainContext->order().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных заказа %1. ")
                .arg(MainContext->order().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "orders", QString("id = %1").arg(MainContext->order().get("id")),
          MainContext->order())) {
    sendLog(QString("Получена ошибка при получении данных заказа %1. ")
                .arg(MainContext->order().get("id")));
    return false;
  }

  return true;
}
