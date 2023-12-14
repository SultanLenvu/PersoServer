#include <QSettings>

#include "General/definitions.h"
#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractReleaseSystem(name, db) {
  loadSettings();
}

TransponderReleaseSystem::~TransponderReleaseSystem() {}

void TransponderReleaseSystem::setContext(
    const ProductionLineContext& context) {
  CurrentProductionLine = context.value("production_line");
  CurrentTransponder = context.value("transponder");
  CurrentBox = context.value("box");
  CurrentPallet = context.value("pallet");
  CurrentOrder = context.value("order");
  CurrentIssuer = context.value("issuer");
  CurrentMasterKeys = context.value("master_keys");
}

ReturnStatus TransponderReleaseSystem::release() {
  ReturnStatus ret;

  if (CurrentProductionLine->get("completed") == "true") {
    sendLog(
        QString("Производственная линия %1 заврешила свою "
                "работу в текущем заказе %2.")
            .arg(CurrentProductionLine->get("id"), CurrentOrder->get("id")));
    return ReturnStatus::CurrentOrderAssembled;
  }

  if (CurrentTransponder->get("box_id") !=
      CurrentProductionLine->get("box_id")) {
    sendLog(QString("Транспондер %1 не содержится в соответствующем боксе %1")
                .arg(CurrentTransponder->get("id"),
                     CurrentProductionLine->get("box_id")));
    return ReturnStatus::SynchronizationError;
  }

  if (CurrentTransponder->get("release_counter").toUInt() > 0) {
    sendLog(
        QString("Транспондер %1 уже был выпущен. Повторный выпуск невозможен.")
            .arg(CurrentTransponder->get("id")));
    return ReturnStatus::TransponderIncorrectRerelease;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::confirmRelease(const QString& ucid) {
  // Проверка того, что транспондер не был выпущен ранее
  if (CurrentTransponder->get("release_counter").toInt() >= 1) {
    sendLog(
        QString("Транспондер %1 был выпущен ранее. Подтверждение невозможно. ")
            .arg(CurrentTransponder->get("id")));
    return ReturnStatus::TransponderIncorrectRerelease;
  }

  // Проверка того, что транспондер ожидает подтверждения
  if (CurrentTransponder->get("awaiting_confirmation") == "false") {
    sendLog(QString("Транспондер %1 не был выпущен ранее.  Подтверждение "
                    "выпуска невозможно. ")
                .arg(CurrentTransponder->get("id")));
    return ReturnStatus::TransponderNotAwaitingConfirmation;
  }

  // Подтверждаем сборку транспондера
  if (!confirmCurrentTransponder(ucid)) {
    sendLog(QString("Получена ошибка при подтвеждении транспондера %1. ")
                .arg(CurrentProductionLine->get("transponder_id")));

    return ReturnStatus::DatabaseQueryError;
  }

  // Ищем новый транспондер для производственной линии
  ReturnStatus ret = searchNextTransponder();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при поиске очередного транспондера "
                    "для производственной линии %1. ")
                .arg(CurrentProductionLine->get("id")));
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::rerelease(const QString& key,
                                                 const QString& value) {
  SqlQueryValues transponder;
  SqlQueryValues newTransponder;

  if (!Database->readRecords(
          "transponders", QString("%1 = '%2'").arg(key, value), transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Транспондера с %1 = '%2' не найден.").arg(key, value));
    return ReturnStatus::TranspoderMissed;
  }

  // Проверка, что транспондер уже был выпущен ранее
  if (transponder.get("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее"
                    "перевыпуск невозможен. ")
                .arg(CurrentTransponder->get("id")));
    return ReturnStatus::TransponderNotReleasedEarlier;
  }

  // Ожидаем подтверждения
  newTransponder.add("awaiting_confirmation", "true");
  if (!Database->updateRecords("transponders",
                               QString("id = %1").arg(transponder.get("id")),
                               newTransponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::confirmRerelease(const QString& key,
                                                        const QString& value,
                                                        const QString& ucid) {
  SqlQueryValues transponder;
  SqlQueryValues newTransponder;

  if (!Database->readRecords(
          "transponders", QString("%1 = '%2'").arg(key, value), transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Транспондера с %1 = '%2' не найден.").arg(key, value));
    return ReturnStatus::TranspoderMissed;
  }

  // Проверка, что транспондер ожидает подтверждения
  if (transponder.get("awaiting_confirmation") != "true") {
    sendLog(QString("Транспондер %1 еще не был перевыпущен, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(transponder.get("id")));
    return ReturnStatus::TransponderNotAwaitingConfirmation;
  }

  // Проверка, что новый UCID отличается от прошлого
  if (transponder.get("ucid") == ucid) {
    sendLog(QString("Новый UCID идентичен предыдущему, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(transponder.get("id")));
    return ReturnStatus::IdenticalUcidError;
  }

  // Сохраняем UCID и увеличиваем счетчик выпусков
  newTransponder.add("awaiting_confirmation", "false");
  newTransponder.add(
      "release_counter",
      QString::number(CurrentTransponder->get("release_counter").toInt() + 1));
  newTransponder.add("ucid", ucid);
  if (!Database->updateRecords("transponders",
                               QString("id = %1").arg(transponder.get("id")),
                               newTransponder)) {
    sendLog(QString("Получена ошибка при сохранении UCID транспондера %1. ")
                .arg(CurrentTransponder->get("id")));
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::rollback() {
  SqlQueryValues newTransponder;
  SqlQueryValues transponder;
  SqlQueryValues newBox;
  SqlQueryValues newProductionLine;

  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::DescendingOrder);

  if (CurrentBox->get("assembled_box") == "0") {
    sendLog(QString("Производственная линия '%1' связана с первым "
                    "транспондером в боксе. Откат невозможен.")
                .arg(CurrentProductionLine->get("id")));
    return ReturnStatus::ProductionLineRollbackLimit;
  }

  // Обновляем данные текущего транспондера
  newTransponder.add("release_counter", "0");
  newTransponder.add("ucid", "NULL");
  newTransponder.add("awaiting_confirmation", "false");
  if (!updateCurrentTransponder(newTransponder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Уменьшаем количество собранных транспондеров в боксе
  newBox.add("assembled_units",
             QString::number(CurrentBox->get("assembled_units").toInt() - 1));
  if (!updateCurrentBox(newBox)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Откатываем текущий транспондер
  QString prevTransponderId =
      QString::number(CurrentTransponder->get("id").toInt() - 1);
  if (!switchCurrentTransponder(prevTransponderId)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем производственную линию
  newProductionLine.add("transponder_id", CurrentTransponder->get("id"));
  if (!updateCurrentProductionLine(newProductionLine)) {
    return ReturnStatus::DatabaseQueryError;
  }

  Database->setRecordMaxCount(0);
  Database->setCurrentOrder(Qt::AscendingOrder);

  return ReturnStatus::NoError;
}

void TransponderReleaseSystem::loadSettings() {
  QSettings settings;
}

void TransponderReleaseSystem::sendLog(const QString& log) const {
  emit const_cast<TransponderReleaseSystem*>(this)->logging(objectName() +
                                                            " - " + log);
}

bool TransponderReleaseSystem::confirmCurrentTransponder(const QString& ucid) {
  SqlQueryValues newTransponder;
  SqlQueryValues newBox;

  // Увеличиваем счетчик выпусков транспондера и сохраняем ucid
  newTransponder.add("awaiting_confirmation", "false");
  newTransponder.add("ucid", ucid);
  newTransponder.add("release_counter", "1");
  if (!updateCurrentTransponder(newTransponder)) {
    return false;
  }

  // Увеличиваем счетчик выпущенных транспондеров в боксе
  newBox.add("assembled_units",
             QString::number(CurrentBox->get("assembled_units").toInt() + 1));
  newBox.add("assembling_end", "NULL");
  if (!updateCurrentBox(newBox)) {
    return false;
  }

  // Если бокс целиком собран
  if (CurrentBox->get("assembled_units").toInt() ==
      CurrentBox->get("quantity").toInt()) {
    // Завершаем сборку бокса
    return completeCurrentBoxAssembly();
  }

  // В противном случае возвращаемся
  return true;
}

bool TransponderReleaseSystem::completeCurrentBoxAssembly(void) {
  SqlQueryValues newBox;
  SqlQueryValues newPallet;

  // Завершаем процесс сборки бокса
  newBox.add("in_process", "false");
  newBox.add("assembling_end", QDateTime::currentDateTime().toString(
                                   POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateCurrentBox(newBox)) {
    return false;
  }

  // Отправляем сигнал о завершении сборки бокса
  std::shared_ptr<QString> boxId(new QString(CurrentBox->get("id")));
  emit boxAssemblyCompleted(boxId);

  // Увеличиваем счетчик выпущенных боксов в паллете
  newPallet.add(
      "assembled_units",
      QString::number(CurrentPallet->get("assembled_units").toInt() + 1));
  newPallet.add("assembling_end", "NULL");
  if (!updateCurrentPallet(newPallet)) {
    return false;
  }

  // Если паллета целиком собрана
  if (CurrentPallet->get("assembled_units").toInt() ==
      CurrentPallet->get("quantity").toInt()) {
    // Завершаем сборку паллеты
    return completeCurrentPalletAssembly();
  }

  // В противном случае возвращаемся
  return true;
}

bool TransponderReleaseSystem::completeCurrentPalletAssembly() {
  SqlQueryValues newPallet;
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки паллеты
  newPallet.add("in_process", "false");
  newPallet.add("assembling_end", QDateTime::currentDateTime().toString(
                                      POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateCurrentPallet(newPallet)) {
    return false;
  }

  // Отправляем сигнал о завершении сборки паллеты
  std::shared_ptr<QString> palletId(new QString(CurrentPallet->get("id")));
  emit palletAssemblyCompleted(palletId);

  // Увеличиваем счетчик выпущенных паллет в заказе
  newOrder.add(
      "assembled_units",
      QString::number(CurrentOrder->get("assembled_units").toInt() + 1));
  newOrder.add("assembling_end", "NULL");
  if (!updateCurrentOrder(newOrder)) {
    return false;
  }

  // Если заказ целиком собран
  if (CurrentOrder->get("assembled_units").toInt() ==
      CurrentOrder->get("quantity").toInt()) {
    // Завершаем сборку заказа
    return completeCurrentOrderAssembly();
  }

  // В противном случае возвращаемся
  return true;
}

bool TransponderReleaseSystem::completeCurrentOrderAssembly() {
  SqlQueryValues newOrder;

  // Установка даты окончания и завершение процесса сборки заказа
  newOrder.add("in_process", "false");
  newOrder.add("assembling_end", QDateTime::currentDateTime().toString(
                                     POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateCurrentOrder(newOrder)) {
    return false;
  }

  // Отправляем сигнал о завершении сборки заказа
  std::shared_ptr<QString> orderId(new QString(CurrentOrder->get("id")));
  emit palletAssemblyCompleted(orderId);

  return true;
}

ReturnStatus TransponderReleaseSystem::searchNextTransponder() {
  SqlQueryValues newProductionLine;
  QString nextTransponderId;
  ReturnStatus ret;

  // Если текущий бокс собран
  if (CurrentBox->get("assembled_units") == CurrentBox->get("quantity")) {
    // Ищем следующий бокс
    ret = searchNextBox();
    if (ret != ReturnStatus::NoError) {
      return ret;
    }

    // Ищем первый транспондер в новом текущем боксе
    SqlQueryValues transponder;
    Database->setRecordMaxCount(1);
    Database->setCurrentOrder(Qt::AscendingOrder);
    if (!Database->readRecords(
            "transponders", QString("box_id = %1").arg(CurrentBox->get("id")),
            transponder)) {
      sendLog(QString("Получена ошибка при выполнении запроса в базу . "));
      return ReturnStatus::DatabaseQueryError;
    }

    nextTransponderId = transponder.get("id");
  } else {
    // В противном случае двигаемся к следующему транспондеру в боксе
    nextTransponderId =
        QString::number(CurrentTransponder->get("id").toInt() + 1);
  }

  // Переключаем текущий транспондер
  if (!switchCurrentTransponder(nextTransponderId)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем производственную линию
  newProductionLine.add("transponder_id", nextTransponderId);
  if (!updateCurrentProductionLine(newProductionLine)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::searchNextBox() {
  SqlQueryValues freeBox;
  QStringList tables{"boxes", "pallets", "orders"};

  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (!Database->readMergedRecords(
          tables,
          QString("boxes.production_line_id = NULL AND orders.id = %2 AND "
                  "boxes.assembled_units < boxes.quantity")
              .arg(CurrentOrder->get("id")),
          freeBox)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (freeBox.isEmpty()) {
    sendLog(QString("В заказе %1 не осталось свободных боксов. "
                    "Производственная линия %1 завершила свою работу.")
                .arg(CurrentProductionLine->get("id")));
    return ReturnStatus::ProductionLineCompleted;
  }

  if (!switchCurrentBox(freeBox.get("id"))) {
    return ReturnStatus::DatabaseQueryError;
  }

  if (!startCurrentBoxAssembly()) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Если новый бокс находится в новой паллете
  if (CurrentPallet->get("id") != freeBox.get("pallet_id")) {
    if (switchCurrentPallet(freeBox.get("pallet_id"))) {
      return ReturnStatus::DatabaseQueryError;
    }

    // Если новая паллета не в процессе сборки
    if (CurrentPallet->get("in_process") == "false") {
      if (!startCurrentPalletAssembly()) {
        return ReturnStatus::DatabaseQueryError;
      }
    }
  }

  return ReturnStatus::NoError;
}

bool TransponderReleaseSystem::startCurrentBoxAssembly() {
  SqlQueryValues newBox;

  newBox.add("in_process", "true");
  newBox.add("assembling_start", QDateTime::currentDateTime().toString(
                                     POSTGRES_TIMESTAMP_TEMPLATE));
  newBox.add("production_line_id", CurrentProductionLine->get("id"));
  if (!updateCurrentBox(newBox)) {
    return false;
  }

  SqlQueryValues newProductionLine;
  newBox.add("box_id", CurrentBox->get("id"));
  if (!updateCurrentProductionLine(newBox)) {
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::startCurrentPalletAssembly() {
  SqlQueryValues newPallet;

  // Запускаем сборку паллеты
  newPallet.add("in_process", "true");
  newPallet.add("assembling_start", QDateTime::currentDateTime().toString(
                                        POSTGRES_TIMESTAMP_TEMPLATE));
  if (!updateCurrentPallet(newPallet)) {
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::switchCurrentTransponder(const QString& id) {
  if (!Database->readRecords("transponders", QString("id = %1").arg(id),
                             *CurrentTransponder)) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(id));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::switchCurrentBox(const QString& id) {
  if (!Database->readRecords("boxes", QString("id = %1").arg(id),
                             *CurrentBox)) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ").arg(id));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::switchCurrentPallet(const QString& id) {
  if (!Database->readRecords("pallets", QString("id = %1").arg(id),
                             *CurrentPallet)) {
    sendLog(
        QString("Получена ошибка при получении данных паллеты %1. ").arg(id));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateCurrentProductionLine(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "production_lines",
          QString("id = %1").arg(CurrentProductionLine->get("id")),
          newValues)) {
    sendLog(
        QString(
            "Получена ошибка при обновлении данных производственной линии %1. ")
            .arg(CurrentProductionLine->get("id")));
    return false;
  }

  if (!Database->readRecords(
          "production_lines",
          QString("id = %1").arg(CurrentProductionLine->get("id")),
          *CurrentProductionLine)) {
    sendLog(
        QString(
            "Получена ошибка при получении данных производственной линии %1. ")
            .arg(CurrentProductionLine->get("id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateCurrentTransponder(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "transponders", QString("id = %1").arg(CurrentTransponder->get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных транспондера %1. ")
                .arg(CurrentTransponder->get("id")));
    return false;
  }

  if (!Database->readRecords(
          "transponders", QString("id = %1").arg(CurrentTransponder->get("id")),
          *CurrentTransponder)) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(CurrentTransponder->get("id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateCurrentBox(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "boxes", QString("id = %1").arg(CurrentBox->get("id")), newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных бокса %1. ")
                .arg(CurrentBox->get("id")));
    return false;
  }

  if (!Database->readRecords("boxes",
                             QString("id = %1").arg(CurrentBox->get("id")),
                             *CurrentBox)) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ")
                .arg(CurrentBox->get("id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateCurrentPallet(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords("pallets",
                               QString("id = %1").arg(CurrentPallet->get("id")),
                               newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных паллеты %1. ")
                .arg(CurrentPallet->get("id")));
    return false;
  }

  if (!Database->readRecords("pallets",
                             QString("id = %1").arg(CurrentPallet->get("id")),
                             *CurrentPallet)) {
    sendLog(QString("Получена ошибка при получении данных паллеты %1. ")
                .arg(CurrentPallet->get("id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateCurrentOrder(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords("orders",
                               QString("id = %1").arg(CurrentOrder->get("id")),
                               newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных заказа %1. ")
                .arg(CurrentOrder->get("id")));
    return false;
  }

  if (!Database->readRecords("orders",
                             QString("id = %1").arg(CurrentOrder->get("id")),
                             *CurrentOrder)) {
    sendLog(QString("Получена ошибка при получении данных заказа %1. ")
                .arg(CurrentOrder->get("id")));
    return false;
  }

  return true;
}
