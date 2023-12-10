#include "transponder_release_system.h"
#include "General/definitions.h"
#include "Log/log_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractReleaseSystem(name, db) {
  loadSettings();

  ContextReady = false;
}

TransponderReleaseSystem::~TransponderReleaseSystem() {}

void TransponderReleaseSystem::setContext(const ProductionContext& context) {
  CurrentProductionLine = context.value("production_line");
  CurrentTransponder = context.value("transponder");
  CurrentBox = context.value("box");
  CurrentPallet = context.value("pallet");
  CurrentOrder = context.value("order");
  CurrentIssuer = context.value("issuer");
  CurrentMasterKeys = context.value("master_keys");

  ContextReady = true;
}

ReturnStatus TransponderReleaseSystem::release() {
  if (!ContextReady) {
    return ReturnStatus::InvalidProductionContext;
  }

  ReturnStatus ret;

  if (CurrentProductionLine->get("transponder_id").isEmpty() ||
      CurrentProductionLine->get("box_id").isEmpty()) {
    sendLog(
        QString(
            "Производственная линия %1 не связана ни с каким транспондером. ")
            .arg(CurrentProductionLine->get("id")));
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

ReturnStatus TransponderReleaseSystem::confirmRelease(
    const StringDictionary& param) {
  if (!ContextReady) {
    return ReturnStatus::InvalidProductionContext;
  }

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
  if (!confirmCurrentTransponder(param.value("ucid"))) {
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

ReturnStatus TransponderReleaseSystem::rerelease(
    const StringDictionary& param) {
  SqlQueryValues transponder;
  SqlQueryValues newTransponder;

  QString pan = param.value("personal_account_number")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  if (!Database->readRecords("transponders", QString("pan = %1").arg(pan),
                             transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Транспондера с PAN = %1 не найден.")
                .arg(param.value("personal_account_number")));
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

ReturnStatus TransponderReleaseSystem::confirmRerelease(
    const StringDictionary& param) {
  SqlQueryValues transponder;
  SqlQueryValues newTransponder;

  QString pan = param.value("personal_account_number")
                    .leftJustified(FULL_PAN_CHAR_LENGTH, QChar('F'));
  if (!Database->readRecords("transponders", QString("pan = %1").arg(pan),
                             transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Транспондера с PAN = %1 не найден.")
                .arg(param.value("personal_account_number")));
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
  if (transponder.get("ucid") == param.value("ucid")) {
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
  newTransponder.add("ucid", param.value("ucid"));
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

  // Ищем предыдущий транспондер
  if (!Database->readRecords("transponders",
                             QString("box_id = %1  AND release_counter > 0")
                                 .arg(CurrentBox->get("id")),
                             transponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (transponder.isEmpty()) {
    sendLog(QString("Производственная линия '%1' связана с первым "
                    "транспондером в боксе. Откат невозможен.")
                .arg(CurrentProductionLine->get("id")));
    return ReturnStatus::ProductionLineRollbackLimit;
  }

  // Уменьшаем количество собранных транспондеров в боксе
  newBox.add("assembled_units",
             QString::number(CurrentBox->get("assembled_units").toInt() - 1));
  newBox.add("assembling_end", "NULL");
  if (!updateCurrentBox(newBox)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем производственную линию
  newProductionLine.add("transponder_id", transponder.get("id"));
  if (!updateCurrentProductionLine(newProductionLine)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем текущий транспондер производственной линии
  if (!Database->readRecords("transponders",
                             QString("id = %1").arg(transponder.get("id")),
                             *CurrentTransponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем данные текущего транспондера
  newTransponder.add("release_counter", "0");
  newTransponder.add("ucid", "NULL");
  newTransponder.add("awaiting_confirmation", "false");
  if (!updateCurrentTransponder(newTransponder)) {
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
  LogSystem::instance()->generate(objectName() + " - " + log);
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
  emit boxAssemblyCompleted(CurrentBox->get("id"));

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
  emit palletAssemblyCompleted(CurrentPallet->get("id"));

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
  emit palletAssemblyCompleted(CurrentOrder->get("id"));

  return true;
}

ReturnStatus TransponderReleaseSystem::searchNextTransponder() {
  SqlQueryValues newTransponder;
  ReturnStatus ret;

  // Ищем невыпущенный транспондер в текущем боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", "0");
  transponderRecord.insert("box_id", CurrentBox->get("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске невыпущенного "
                    "транспондера в боксе %1. ")
                .arg(transponderRecord.get("box_id")));
    return NextTransponderNotFound;
  }

  // Если свободный транспондер в текущем боксе найден
  if (!transponderRecord.isEmpty()) {
    // Связываем текущую линию производства с найденным транспондером
    return linkCurrentProductionLine(transponderRecord.get("id"));
  }

  sendLog(
      QString("В боксе %1 кончились свободные транспондеры. Поиск следующего "
              "бокса для сборки.  ")
          .arg(CurrentBox->get("id")));

  // В противном случае ищем свободный бокс в текущей паллете
  boxRecord.insert("id", "");
  boxRecord.insert("ready_indicator", "false");
  boxRecord.insert("in_process", "false");
  boxRecord.insert("pallet_id", CurrentPallet->get("id"));
  if (!Database->getRecordByPart("boxes", boxRecord)) {
    sendLog(
        QString("Получена ошибка при поиске свободного бокса в паллете %1. ")
            .arg(boxRecord.get("pallet_id")));
    return NextTransponderNotFound;
  }

  // Если свободный бокс найден
  if (!boxRecord.isEmpty()) {
    sendLog(QString("Запуск сборки бокса %1.  ").arg(boxRecord.get("id")));
    // Запускаем сборку бокса
    return startBoxAssembling(boxRecord.get("id"));
  }

  sendLog(QString("В паллете %1 кончились свободные боксы. Поиск следующей "
                  "паллеты для сборки.  ")
              .arg(CurrentPallet->get("id")));

  QStringList tables;
  tables.append("boxes");
  tables.append("pallets");
  tables.append("orders");

  QStringList foreignKeys;
  foreignKeys.append("pallet_id");
  foreignKeys.append("order_id");

  mergedRecord.insert("boxes.id", "");
  mergedRecord.insert("boxes.pallet_id", "");
  mergedRecord.insert("boxes.ready_indicator", "false");
  mergedRecord.insert("boxes.in_process", "false");
  mergedRecord.insert("pallets.order_id", CurrentOrder.get("id"));

  if (!Database->getMergedRecordByPart(tables, foreignKeys, mergedRecord)) {
    sendLog(QString("Получена ошибка при поиске свободного бокса в заказе %1. ")
                .arg(CurrentOrder.get("id")));
    return NextTransponderNotFound;
  }

  //  // Если свободных боксов в текущей паллете не найдено
  //  // Ищем свободную паллету или паллету в процессе сборки в текущем заказе
  //  palletRecord.insert("id", "");
  //  palletRecord.insert("in_process", "false");
  //  palletRecord.insert("ready_indicator", "false");
  //  palletRecord.insert("order_id", CurrentOrder.get("id"));
  //  if (!Database->getRecordByPart("pallets", palletRecord)) {
  //    sendLog(
  //        QString("Получена ошибка при поиске свободной паллеты в заказе %1.
  //        ")
  //            .arg(palletRecord.get("order_id")));
  //    return NextTransponderNotFound;
  //  }

  //  // Если свободная паллета в текущем заказе найдена
  //  if (!palletRecord.isEmpty()) {
  //    sendLog(
  //        QString("Запуск сборки паллеты %1.
  //        ").arg(palletRecord.get("id")));
  //    // Запускаем сборку паллеты
  //    return startPalletAssembling(palletRecord.get("id"));
  //  }

  // Если свободный бокс найден
  if (!mergedRecord.isEmpty()) {
    // Ищем данные его паллеты
    palletRecord.insert("id", mergedRecord.get("pallet_id"));
    palletRecord.insert("in_process", "");
    if (!Database->getRecordById("pallets", palletRecord)) {
      sendLog(QString("Получена ошибка при поиске данных паллеты %1.")
                  .arg(mergedRecord.get("pallet_id")));
      return NextTransponderNotFound;
    }

    if (palletRecord.get("in_process") == "true") {
      return startBoxAssembling(mergedRecord.get("id"));
    } else {
      return startPalletAssembling(palletRecord.get("id"));
    }
  }

  // Если свободной паллеты в текущем заказе не найдено
  sendLog(
      QString("В заказе %1 закончились свободные паллеты. "
              "Производственная линия %2 останавливается. ")
          .arg(palletRecord.get("order_id"), CurrentProductionLine->get("id")));
  if (stopCurrentProductionLine() != Completed) {
    return ProductionLineStopError;
  }

  return CurrentOrderRunOut;
}

ReturnStatus TransponderReleaseSystem::searchNextBox() {}

ReturnStatus TransponderReleaseSystem::searchNextPallet() {}

ReturnStatus TransponderReleaseSystem::startBoxAssembling(const QString& id) {
  QHash<QString, QString> transponderRecord;
  QHash<QString, QString> boxRecord;

  boxRecord.insert("id", id);
  boxRecord.insert("in_process", "true");
  boxRecord.insert("assembling_start", QDateTime::currentDateTime().toString(
                                           POSTGRES_TIMESTAMP_TEMPLATE));
  boxRecord.insert("production_line_id", CurrentProductionLine->get("id"));
  if (!Database->updateRecordById("boxes", boxRecord)) {
    sendLog(
        QString("Получена ошибка при запуске сборки бокса %1 в паллете %2. ")
            .arg(boxRecord.get("id"), boxRecord.get("pallet_id")));
    return StartBoxAssemblingError;
  }

  // Ищем в запущенном боксе первый невыпущенный транспондер
  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", "0");
  transponderRecord.insert("box_id", boxRecord.get("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске невыпущенного "
                    "транспондера в боксе %1. ")
                .arg(transponderRecord.get("box_id")));
    return NextTransponderNotFound;
  }

  // Связываем текущую линию производства с найденным транспондером
  return linkCurrentProductionLine(transponderRecord.get("id"));
}

ReturnStatus TransponderReleaseSystem::startPalletAssembling(
    const QString& id) {
  QHash<QString, QString> boxRecord;
  QHash<QString, QString> palletRecord;

  // Запускаем сборку паллеты
  palletRecord.insert("id", id);
  palletRecord.insert("in_process", "true");
  palletRecord.insert("assembling_start", QDateTime::currentDateTime().toString(
                                              POSTGRES_TIMESTAMP_TEMPLATE));
  if (!Database->updateRecordById("pallets", palletRecord)) {
    sendLog(
        QString("Получена ошибка при запуске сборки паллеты %1 в заказе %2. ")
            .arg(palletRecord.get("id"), palletRecord.get("order_id")));
    return StartPalletAssemblingError;
  }

  // Ищем первый бокс в найденной свободной паллете
  boxRecord.insert("id", "");
  boxRecord.insert("in_process", "false");
  boxRecord.insert("ready_indicator", "false");
  boxRecord.insert("pallet_id", palletRecord.get("id"));
  if (!Database->getRecordByPart("boxes", boxRecord)) {
    sendLog(QString("Получена ошибка при поиске несобранного "
                    "бокса в паллете %1. ")
                .arg(id));
    return NextTransponderNotFound;
  }

  if (boxRecord.isEmpty()) {
    sendLog(QString("Не найдено свободных боксов в паллете %1. ").arg(id));
    return FreeBoxMissed;
  }

  return startBoxAssembling(boxRecord.get("id"));
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
