#include "transponder_release_system.h"
#include "General/definitions.h"
#include "Log/log_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractReleaseSystem(name, db) {
  loadSettings();
}

TransponderReleaseSystem::~TransponderReleaseSystem() {}

ReturnStatus TransponderReleaseSystem::release(const StringDictionary& param) {
  ReturnStatus ret;
  SqlQueryValues transponderNew;

  // Получаем текущий контекст
  ret = getCurrentContext(param);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(param.get("login")));
    return ret;
  }

  if (CurrentTransponder.get("box_id") != CurrentProductionLine.get("box_id")) {
    sendLog(QString("Транспондер %1 не содержится в соответствующем боксе %1")
                .arg(CurrentTransponder.get("id"),
                     CurrentProductionLine.get("box_id")));
    return ReturnStatus::SynchronizationError;
  }

  if (CurrentTransponder.get("release_counter").toUInt() > 0) {
    sendLog(
        QString("Транспондер %1 уже был выпущен. Повторный выпуск невозможен.")
            .arg(CurrentTransponder.get("id")));
    return ReturnStatus::TransponderWasReleasedEarlier;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::confirmRelease(
    const StringDictionary& param) {
  ReturnStatus ret;
  // Получаем текущий контекст
  ret = getCurrentContext(param);
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(param.value("login")));
    return ret;
  }

  // Проверка того, что транспондер не был выпущен ранее
  if (CurrentTransponder.get("release_counter").toInt() >= 1) {
    sendLog(
        QString("Транспондер %1 был выпущен ранее. Подтверждение невозможно. ")
            .arg(CurrentTransponder.get("id")));
    return ReturnStatus::TransponderWasReleasedEarlier;
  }

  // Проверка того, что транспондер ожидает подтверждения
  if (CurrentTransponder.get("awaiting_confirmation") == "false") {
    sendLog(QString("Транспондер %1 не был выпущен ранее.  Подтверждение "
                    "выпуска невозможно. ")
                .arg(CurrentTransponder.get("id")));
    return ReturnStatus::TransponderAwaitingConfirmationError;
  }

  // Подтверждаем сборку транспондера
  if (!confirmCurrentTransponder(parameters->value("ucid"))) {
    sendLog(QString("Получена ошибка при подтвеждении транспондера %1. ")
                .arg(CurrentProductionLine.get("transponder_id")));

    return ReturnStatus::DatabaseQueryError;
  }

  // Ищем новый транспондер для производственной линии
  if (!searchNextTransponderForCurrentProductionLine()) {
    sendLog(QString("Получена ошибка при поиске очередного транспондера "
                    "для производственной линии %1. ")
                .arg(CurrentProductionLine.get("id")));
    *status = DatabaseQueryError;

    return;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::rerelease(
    const StringDictionary& param) {
  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));

    return;
  }

  // Проверка, что транспондер уже был выпущен ранее
  if (CurrentTransponder.get("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее"
                    "перевыпуск невозможен. ")
                .arg(CurrentTransponder.get("id")));
    *status = TransponderNotReleasedEarlier;
  }

  // Ожидаем подтверждения
  CurrentTransponder.insert("awaiting_confirmation", "true");
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при включении ожидания подтверждения "
                    "транспондера %1. ")
                .arg(CurrentTransponder.get("id")));
    *status = DatabaseQueryError;

    return;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::confirmRerelease(
    const StringDictionary& param) {
  QHash<QString, QString> transponderRecord;
  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));

    return;
  }

  // Проверка, что транспондер не был выпущен ранее
  if (CurrentTransponder.get("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(CurrentTransponder.get("id")));
    *status = TransponderNotReleasedEarlier;
    return;
  }

  // Проверка, что транспондер ожидает подтверждения
  if (CurrentTransponder.get("awaiting_confirmation") != "true") {
    sendLog(QString("Транспондер %1 еще не был перевыпущен, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(CurrentTransponder.get("id")));
    *status = AwaitingConfirmationError;
    return;
  }

  // Проверка, что новый UCID отличается от прошлого
  transponderRecord.insert("ucid", parameters->value("ucid"));
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString(
        "Получена ошибка при проверке уникальности полученного UCID. "));
    *status = DatabaseQueryError;
    return;
  }
  if (!transponderRecord.isEmpty()) {
    sendLog(QString("Полученный UCID уже существует в базе, повторный выпуск "
                    "транспондера %1 невозможен. ")
                .arg(CurrentTransponder.get("id")));
    *status = IdenticalUcidError;
    return;
  }

  // Сохраняем UCID и увеличиваем счетчик выпусков
  CurrentTransponder.insert("awaiting_confirmation", "false");
  CurrentTransponder.insert(
      "release_counter",
      QString::number(CurrentTransponder.get("release_counter").toInt() + 1));
  CurrentTransponder.insert("ucid", parameters->value("ucid"));
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при сохранении UCID транспондера %1. ")
                .arg(CurrentTransponder.get("id")));
    *status = DatabaseQueryError;

    return;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::rollback(const StringDictionary& param) {
  QHash<QString, QString> transponderRecord;

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));

    return;
  }

  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", ">0");
  transponderRecord.insert("box_id", CurrentBox.get("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord, false)) {
    sendLog(QString("Получена ошибка при поиске предыдущего транспондера "
                    "производственной линии в боксе %1. ")
                .arg(transponderRecord.get("box_id")));

    *status = DatabaseQueryError;

    return;
  }

  if (transponderRecord.isEmpty()) {
    sendLog(QString("Производственная линия '%1' связана с первым "
                    "транспондером в боксе. Откат невозможен.")
                .arg(CurrentProductionLine.get("id")));

    *status = ProductionLineRollbackLimitError;

    return;
  }

  transponderRecord.insert("release_counter", "0");
  transponderRecord.insert("ucid", "NULL");
  transponderRecord.insert("awaiting_confirmation", "false");
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при возврате транспондера %1.")
                .arg(transponderRecord.get("id")));

    *status = DatabaseQueryError;

    return;
  }

  CurrentBox.insert(
      "assembled_units",
      QString::number(CurrentBox.get("assembled_units").toInt() - 1));
  CurrentBox.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("boxes", CurrentBox)) {
    sendLog(QString("Получена ошибка при уменьшении количества собранных "
                    "транспондеров в боксе '%1'. ")
                .arg(transponderRecord.get("box_id")));

    *status = DatabaseQueryError;

    return;
  }

  CurrentProductionLine.insert("transponder_id", transponderRecord.get("id"));
  if (!Database->updateRecordById("production_lines", CurrentProductionLine)) {
    sendLog(
        QString("Получена ошибка при связывании производственной линии %1 "
                "с транспондером %2. ")
            .arg(CurrentProductionLine.get("id"), transponderRecord.get("id")));

    *status = DatabaseQueryError;

    return;
  }

  return ReturnStatus::NoError;
}

void TransponderReleaseSystem::loadSettings() {
  QSettings settings;
}

void TransponderReleaseSystem::sendLog(const QString& log) const {
  LogSystem::instance()->generate(objectName() + " - " + log);
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::confirmCurrentTransponder(const QString& ucid) {
  // Увеличиваем счетчик выпусков транспондера и сохраняем ucid
  CurrentTransponder.insert("awaiting_confirmation", "false");
  CurrentTransponder.insert("ucid", ucid);
  CurrentTransponder.insert(
      "release_counter",
      QString::number(CurrentTransponder.get("release_counter").toInt() + 1));
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпусков "
                    "транспондера %1. ")
                .arg(CurrentTransponder.get("id")));
    return DatabaseQueryError;
  }

  // Подтверждаем сборку бокса
  return confirmCurrentBox();
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::confirmCurrentBox(void) {
  // Увеличиваем счетчик выпущенных транспондеров в боксе
  CurrentBox.insert(
      "assembled_units",
      QString::number(CurrentBox.get("assembled_units").toInt() + 1));
  CurrentBox.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("boxes", CurrentBox)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "транспондеров в "
                    "боксе %1. ")
                .arg(CurrentBox.get("id")));
    return DatabaseQueryError;
  }

  // Если бокс целиком собран
  if (CurrentBox.get("assembled_units").toInt() ==
      CurrentBox.get("quantity").toInt()) {
    // Завершаем процесса сборки бокса
    CurrentBox.insert("ready_indicator", "true");
    CurrentBox.insert("in_process", "false");
    CurrentBox.insert("assembling_end", QDateTime::currentDateTime().toString(
                                            POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("boxes", CurrentBox)) {
      sendLog(QString("Получена ошибка при завершении сборки бокса %1. ")
                  .arg(CurrentBox.get("id")));
      return DatabaseQueryError;
    }

    // Собираем данные о боксе и отправляем сигнал о завершении сборки бокса
    QHash<QString, QString> boxData;
    AbstractStickerPrinter::ReturnStatus status;
    generateBoxData(&boxData);
    emit boxAssemblingFinished(&boxData, &status);
    if (status != AbstractStickerPrinter::Completed) {
      emit failed(BoxStickerPrintError);
      return BoxStickerPrintError;
    }

    // Подтверждаем сборку паллеты
    return confirmCurrentPallet();
  }

  return Completed;
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::confirmCurrentPallet() {
  // Увеличиваем счетчик выпущенных боксов в паллете
  CurrentPallet.insert(
      "assembled_units",
      QString::number(CurrentPallet.get("assembled_units").toInt() + 1));
  CurrentPallet.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("pallets", CurrentPallet)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "боксов в "
                    "паллете %1. ")
                .arg(CurrentPallet.get("id")));
    return DatabaseQueryError;
  }

  // Если паллета целиком собрана
  if (CurrentPallet.get("assembled_units").toInt() ==
      CurrentPallet.get("quantity").toInt()) {
    // Установка даты окончания и завершение процесса сборки паллеты
    CurrentPallet.insert("ready_indicator", "true");
    CurrentPallet.insert("in_process", "false");
    CurrentPallet.insert(
        "assembling_end",
        QDateTime::currentDateTime().toString(POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("pallets", CurrentPallet)) {
      sendLog(
          QString(
              "Получена ошибка при установке завершении сборки паллеты %1. ")
              .arg(CurrentPallet.get("id")));
      return DatabaseQueryError;
    }

    // Собираем данные о паллете и отправляем сигнал о завершении сборки
    // паллеты
    QHash<QString, QString> palletData;
    AbstractStickerPrinter::ReturnStatus status;
    generatePalletData(&palletData);
    emit palletAssemblingFinished(&palletData, &status);
    if (status != AbstractStickerPrinter::Completed) {
      emit failed(PalletStickerPrintError);
      return PalletStickerPrintError;
    }

    // Подтверждаем сборку в заказе
    return confirmCurrentOrder();
  }

  return Completed;
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::confirmCurrentOrder() {
  // Увеличиваем счетчик выпущенных паллет в заказе
  CurrentOrder.insert(
      "assembled_units",
      QString::number(CurrentOrder.get("assembled_units").toInt() + 1));
  CurrentOrder.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("orders", CurrentOrder)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "паллет в "
                    "заказе %1. ")
                .arg(CurrentOrder.get("id")));
    return DatabaseQueryError;
  }

  if (CurrentOrder.get("assembled_units").toInt() ==
      CurrentOrder.get("quantity").toInt()) {
    // Установка даты окончания и завершение процесса сборки заказа
    CurrentOrder.insert("ready_indicator", "true");
    CurrentOrder.insert("in_process", "false");
    CurrentOrder.insert("assembling_end", QDateTime::currentDateTime().toString(
                                              POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("orders", CurrentOrder)) {
      sendLog(QString("Получена ошибка при завершении сборки заказа %1. ")
                  .arg(CurrentOrder.get("id")));
      return DatabaseQueryError;
    }
  }

  return Completed;
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::searchNextTransponderForCurrentProductionLine() {
  QHash<QString, QString> transponderRecord;
  QHash<QString, QString> boxRecord;
  QHash<QString, QString> palletRecord;
  QHash<QString, QString> mergedRecord;
  ReturnStatus ret;

  // Ищем невыпущенный транспондер в текущем боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", "0");
  transponderRecord.insert("box_id", CurrentBox.get("id"));
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
          .arg(CurrentBox.get("id")));

  // В противном случае ищем свободный бокс в текущей паллете
  boxRecord.insert("id", "");
  boxRecord.insert("ready_indicator", "false");
  boxRecord.insert("in_process", "false");
  boxRecord.insert("pallet_id", CurrentPallet.get("id"));
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
              .arg(CurrentPallet.get("id")));

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
          .arg(palletRecord.get("order_id"), CurrentProductionLine.get("id")));
  if (stopCurrentProductionLine() != Completed) {
    return ProductionLineStopError;
  }

  return CurrentOrderRunOut;
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::startBoxAssembling(const QString& id) {
  QHash<QString, QString> transponderRecord;
  QHash<QString, QString> boxRecord;

  boxRecord.insert("id", id);
  boxRecord.insert("in_process", "true");
  boxRecord.insert("assembling_start", QDateTime::currentDateTime().toString(
                                           POSTGRES_TIMESTAMP_TEMPLATE));
  boxRecord.insert("production_line_id", CurrentProductionLine.get("id"));
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

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::startPalletAssembling(const QString& id) {
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

void TransponderReleaseSystem::generateFirmwareSeed(
    StringDictionary& seed) const {}
