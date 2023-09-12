#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(QObject* parent)
    : QObject(parent) {
  setObjectName("TransponderReleaseSystem");

  loadSettings();

  createDatabaseController();

  OrderAssembled = false;
  connect(this, &TransponderReleaseSystem::orderAssemblingCompleted, this,
          &TransponderReleaseSystem::on_OrderAssemblingCompleted_slot);

  FreeTranspondersOut = false;
  connect(this, &TransponderReleaseSystem::orderTranspondersOut, this,
          &TransponderReleaseSystem::on_FreeTranspondersOut_slot);
}

bool TransponderReleaseSystem::start() {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    emit logging("Не удалось установить соединение с базой данных. ");
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::stop() {
  emit logging("Отключение от базы данных. ");
  if (!Database->disconnect()) {
    emit logging("Не удалось отключить соединение с базой данных. ");
    return false;
  }

  return true;
}

void TransponderReleaseSystem::beginAssemblingNewOrder(const QString& id) {}

void TransponderReleaseSystem::beginAssemblingNextOrder() {}

void TransponderReleaseSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();

  Database->applySettings();
}

void TransponderReleaseSystem::release(const QMap<QString, QString>* searchData,
                                       QMap<QString, QString>* resultData,
                                       ReturnStatus* status) {
  QMap<QString, QString> productionLineRecord;
  QMap<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    return;
  }

  // Получаем данные о производственной линии
  productionLineRecord.insert("login", searchData->value("login"));
  productionLineRecord.insert("password", searchData->value("password"));
  productionLineRecord.insert("transponder_id", "");
  productionLineRecord.insert("id", "");
  productionLineRecord.insert("active", "");
  if (!Database->getRecordByPart("production_lines", productionLineRecord)) {
    emit logging(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'. ")
            .arg(searchData->value("login")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Если производственная линия не активна, то возвращаемся
  if (productionLineRecord.value("active") == "false") {
    *status = ProductionLineNotActive;
    Database->abortTransaction();
    return;
  }

  // Получаем данные о текущем транспондере
  transponderRecord.insert("id", productionLineRecord.value("transponder_id"));
  transponderRecord.insert("release_counter", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных транспондера '%1'. ")
            .arg(searchData->value("login")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Если транспондер уже был выпущен, то повторный выпуск недопустим.
  // Перевыпуск должен осуществляться только соответствующей функцией
  if (transponderRecord.value("release_counter").toInt() != 0) {
    emit logging(
        QString("Транспондер %1 уже был выпущен, повторный выпуск невозможен. ")
            .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Сохраняем полученный UCID и ожидаем подтверждения
  transponderRecord.insert("id", productionLineRecord.value("transponder_id"));
  transponderRecord.insert("ucid", searchData->value("ucid"));
  transponderRecord.insert("awaiting_confirmation", "true");
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при сохранении UCID транспондера %1. ")
            .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Выгружаем всю информацию о выпускаемом транспондере
  if (!getTranponderData("id", productionLineRecord.value("transponder_id"),
                         resultData)) {
    emit logging(
        "Получена ошибка при получении объединенных данных о транспондере. ");
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
}

void TransponderReleaseSystem::confirmRelease(
    const QMap<QString, QString>* searchData,
    ReturnStatus* status) {
  QMap<QString, QString> productionLineRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = Failed;
    return;
  }

  // Запрашиваем данные о производственной линии
  productionLineRecord.insert("login", searchData->value("login"));
  productionLineRecord.insert("password", searchData->value("password"));
  productionLineRecord.insert("transponder_id", "");
  productionLineRecord.insert("id", "");
  productionLineRecord.insert("active", "");
  if (!Database->getRecordByPart("production_lines", productionLineRecord)) {
    emit logging("Получена ошибка при поиске данных производственной линии. ");
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Если производственная линия не активна, то возвращаемся
  if (productionLineRecord.value("active") == "false") {
    *status = ProductionLineNotActive;
    Database->abortTransaction();
    return;
  }

  // Подтверждаем сборку транспондера
  if (!confirmTransponder(productionLineRecord.value("transponder_id"))) {
    emit logging(QString("Получена ошибка при подтвеждении транспондера %1. ")
                     .arg(productionLineRecord.value("transponder_id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Ищем новый транспондер для производственной линии
  if (!searchNextTransponderForAssembling(&productionLineRecord)) {
    emit logging(QString("Получена ошибка при поиске очередного транспондера "
                         "для производственной линии %1. ")
                     .arg(productionLineRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = Failed;
    return;
  }
  *status = Success;
}

void TransponderReleaseSystem::rerelease(
    const QMap<QString, QString>* searchData,
    QMap<QString, QString>* resultData,
    ReturnStatus* status) {
  QMap<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    return;
  }

  // Получаем данные о перевыпускаемом транспондере
  transponderRecord.insert("id", searchData->value("id"));
  transponderRecord.insert("payment_means", searchData->value("payment_means"));
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("ucid", "");
  transponderRecord.insert("box_id", "");
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных транспондера '%1'. ")
            .arg(searchData->value("login")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Осуществляем логические проверки
  if (!checkRerelease(transponderRecord, *searchData)) {
    emit logging(
        QString("Получена логическая ошибка при перевыпуске транспондера %1. ")
            .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Ожидаем подтверждения
  transponderRecord.insert("awaiting_confirmation", "true");
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    emit logging(QString("Получена ошибка при включении ожидания подтверждения "
                         "транспондера %1. ")
                     .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Выгружаем всю информацию о перевыпущенном транспондере
  if (!getTranponderData("id", transponderRecord.value("id"), resultData)) {
    emit logging(
        "Получена ошибка при получении объединенных данных о транспондере. ");
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
}

void TransponderReleaseSystem::confirmRerelease(
    const QMap<QString, QString>* searchData,
    ReturnStatus* status) {
  QMap<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    return;
  }

  // Получаем данные о перевыпускаемом транспондере
  transponderRecord.insert("id", searchData->value("id"));
  transponderRecord.insert("payment_means", searchData->value("payment_means"));
  transponderRecord.insert("ucid", "");
  transponderRecord.insert("release_counter", "");
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных транспондера '%1'. ")
            .arg(searchData->value("login")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Проверка, что транспондер ожидает подтверждения
  if (transponderRecord.value("awaiting_confirmation") !=
      searchData->value("true")) {
    emit logging(QString("Транспондер %1 не был перевыпущен, "
                         "переподтверждение перевыпуска невозможно. ")
                     .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Осуществляем логические проверки
  if (!checkRerelease(transponderRecord, *searchData)) {
    emit logging(
        QString("Получена логическая ошибка при перевыпуске транспондера %1. ")
            .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Сохраняем UCID и увеличиваем счетчик выпусков
  transponderRecord.insert("awaiting_confirmation", "false");
  transponderRecord.insert(
      "release_counter",
      QString::number(transponderRecord.value("release_counter").toInt() + 1));
  transponderRecord.insert("ucid", searchData->value("ucid"));
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при сохранении UCID транспондера %1. ")
            .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
}

void TransponderReleaseSystem::search(const QMap<QString, QString>* searchData,
                                      QMap<QString, QString>* resultData,
                                      ReturnStatus* status) {
  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    return;
  }

  if (!getTranponderData(searchData->constBegin().key(),
                         searchData->constBegin().value(), resultData)) {
    emit logging("Получена ошибка при поиске транспондера. ");
    *status = Failed;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
}

void TransponderReleaseSystem::createDatabaseController() {
  Database = new PostgresController(this, "ReleaserConnection");
  connect(Database, &IDatabaseController::logging, this,
          &TransponderReleaseSystem::proxyLogging);
}

void TransponderReleaseSystem::loadSettings() {}

bool TransponderReleaseSystem::getTranponderData(
    const QString& searchKey,
    const QString& searchValue,
    QMap<QString, QString>* resultData) {
  QStringList tables;
  QStringList foreignKeys;

  tables.append("transponders");
  tables.append("boxes");
  tables.append("pallets");
  tables.append("orders");
  tables.append("issuers");
  foreignKeys.append("box_id");
  foreignKeys.append("pallet_id");
  foreignKeys.append("order_id");
  foreignKeys.append("issuer_id");
  resultData->insert("model", "");
  resultData->insert("release_counter", "");
  resultData->insert("awaiting_confirmation", "");
  resultData->insert("ucid", "");
  resultData->insert("group_id", "");
  resultData->insert("payment_means", "");
  resultData->insert("efc_context_mark", "");
  resultData->insert("full_personalization", "");
  resultData->insert("boxes.in_process", "");
  resultData->insert("transponders." + searchKey, searchValue);

  if (!Database->getMergedRecordById(tables, foreignKeys, *resultData)) {
    return false;
  }
  return true;
}

bool TransponderReleaseSystem::checkRerelease(
    const QMap<QString, QString>& transponderRecord,
    const QMap<QString, QString>& searchData) {
  QMap<QString, QString> productionLineRecord;
  QMap<QString, QString> boxRecord;

  // Проверка, что транспондер найден
  if (transponderRecord.isEmpty()) {
    emit logging(QString("Транспондер не был найден, перевыпуск невозможен. "));
    return false;
  }

  // Проверка, что транспондер уже был выпущен ранее
  if (transponderRecord.value("release_counter").toInt() <= 0) {
    emit logging(
        QString(
            "Транспондер %1 еще не был выпущен, повторный выпуск невозможен. ")
            .arg(transponderRecord.value("id")));
    return false;
  }

  // Проверка, что новый UCID отличается от прошлого
  if (transponderRecord.value("ucid") == searchData.value("ucid")) {
    emit logging(QString("Новый UCID идентичен прошлому, повторный выпуск "
                         "транспондера %1 невозможен. ")
                     .arg(transponderRecord.value("id")));
    return false;
  }

  // Запрашиваем данные о производственной линии
  productionLineRecord.insert("login", searchData.value("login"));
  productionLineRecord.insert("password", searchData.value("password"));
  productionLineRecord.insert("id", "");
  if (!Database->getRecordByPart("production_lines", productionLineRecord)) {
    emit logging(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'. ")
            .arg(productionLineRecord.value("login")));
    return false;
  }

  // Запрашиваем данные о боксе
  boxRecord.insert("id", transponderRecord.value("box_id"));
  boxRecord.insert("production_line_id", "");
  if (!Database->getRecordByPart("boxes", boxRecord)) {
    emit logging(QString("Получена ошибка при поиске данных бокса %1. ")
                     .arg(transponderRecord.value("box_id")));
    return false;
  }

  // Проверка того, что транспондер перевыпускается той же производственной
  // линией
  if (boxRecord.value("production_line_id") !=
      productionLineRecord.value("id")) {
    emit logging(
        QString("Перевыпуск транспондера сторонней производственной линией "
                "невозможен. "));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::confirmTransponder(
    const QString& transponderId) const {
  QMap<QString, QString> transponderRecord;

  // Запрашиваем данные транспондера
  transponderRecord.insert("id", transponderId);
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("box_id", "");
  transponderRecord.insert("awaiting_confirmation", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    emit logging(QString("Получена ошибка при поиске данных транспондера %1. ")
                     .arg(transponderId));
    return false;
  }

  // Проверка того, что транспондер не был выпущен ранее
  if (transponderRecord.value("release_counter") != "0") {
    emit logging(
        QString("Транспондер %1 был выпущен ранее.  Подтверждение невозможно. ")
            .arg(transponderId));
    return false;
  }

  // Проверка того, что транспондер ожидает подтверждения
  if (transponderRecord.value("awaiting_confirmation") != "true") {
    emit logging(
        QString("Транспондер %1 не был выпущен.  Подтверждение невозможно. ")
            .arg(transponderId));
    return false;
  }

  // Подтверждаем транспондер и увеличиваем счетчик выпусков транспондера
  transponderRecord.insert("awaiting_confirmation", "false");
  transponderRecord.insert(
      "release_counter",
      QString::number(transponderRecord.value("release_counter").toInt() + 1));
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    emit logging(QString("Получена ошибка при увеличении счетчика выпусков "
                         "транспондера %1. ")
                     .arg(transponderId));
    return false;
  }

  // Испускаем сигнал о конце сборки транспондера
  emit transponderAssemblingCompleted(&transponderRecord);

  if (!confirmBox(transponderRecord.value("box_id"))) {
    emit logging(QString("Получена ошибка при подтверждении бокса %1. ")
                     .arg(transponderRecord.value("box_id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::confirmBox(const QString& boxId) const {
  QMap<QString, QString> boxRecord;

  // Получаем данные о боксе
  boxRecord.insert("id", boxId);
  boxRecord.insert("assembled_units", "");
  boxRecord.insert("capacity", "");
  boxRecord.insert("assembling_start", "");
  boxRecord.insert("assembling_end", "");
  boxRecord.insert("pallet_id", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(boxId));
    return false;
  }

  // Увеличиваем счетчик выпущенных транспондеров в боксе
  boxRecord.insert(
      "assembled_units",
      QString::number(boxRecord.value("assembled_units").toInt() + 1));
  if (!Database->updateRecordById("boxes", boxRecord)) {
    emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                         "транспондеров в "
                         "боксе %1. ")
                     .arg(boxId));
    return false;
  }

  // Если бокс целиком собран
  if (boxRecord.value("assembled_units").toInt() ==
      boxRecord.value("capacity").toInt()) {
    // Завершаем процесса сборки бокса
    boxRecord.insert("ready_indicator", "true");
    boxRecord.insert("in_process", "false");
    boxRecord.insert("assembling_end",
                     QDateTime::currentDateTime().toString(TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("boxes", boxRecord)) {
      emit logging(
          QString("Получена ошибка при установке даты начала сборки бокса %1. ")
              .arg(boxId));
      return false;
    }

    // Испускаем сигнал о конце сборки бокса
    emit boxAssemblingCompleted(&boxRecord);

    // Подтверждаем сборку в палете
    if (!confirmPallet(boxRecord.value("pallet_id"))) {
      emit logging(QString("Получена ошибка при подтверждении паллеты %1. ")
                       .arg((boxRecord.value("pallet_id"))));
      return false;
    }
  }

  return true;
}

bool TransponderReleaseSystem::confirmPallet(const QString& id) const {
  QMap<QString, QString> palletRecord;

  // Получаем данные о паллете
  palletRecord.insert("id", id);
  palletRecord.insert("assembled_units", "");
  palletRecord.insert("capacity", "");
  palletRecord.insert("assembling_start", "");
  palletRecord.insert("assembling_end", "");
  palletRecord.insert("order_id", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  // Увеличиваем счетчик выпущенных боксов в паллете
  palletRecord.insert(
      "assembled_units",
      QString::number(palletRecord.value("assembled_units").toInt() + 1));
  if (!Database->updateRecordById("pallets", palletRecord)) {
    emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                         "боксов в "
                         "паллете %1. ")
                     .arg(id));
    return false;
  }

  // Если паллета целиком собрана
  if (palletRecord.value("assembled_units").toInt() ==
      palletRecord.value("capacity").toInt()) {
    // Установка даты окончания и завершение процесса сборки паллеты
    palletRecord.insert("ready_indicator", "true");
    palletRecord.insert("in_process", "false");
    palletRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                              TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("pallets", palletRecord)) {
      emit logging(
          QString(
              "Получена ошибка при установке даты начала сборки паллеты %1. ")
              .arg(id));
      return false;
    }

    // Испускаем сигнал о конце сборки паллеты
    emit palletAssemblingCompleted(&palletRecord);

    // Подтверждаем сборку в заказе
    if (!confirmOrder(palletRecord.value("order_id"))) {
      emit logging(QString("Получена ошибка при подтверждении заказа %1. ")
                       .arg((palletRecord.value("order_id"))));
      return false;
    }
  }

  return true;
}

bool TransponderReleaseSystem::confirmOrder(const QString& id) const {
  QMap<QString, QString> orderRecord;

  // Получаем данные о заказе
  orderRecord.insert("id", id);
  orderRecord.insert("assembled_units", "");
  orderRecord.insert("capacity", "");
  orderRecord.insert("assembling_start", "");
  orderRecord.insert("assembling_end", "");
  if (!Database->getRecordById("orders", orderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  // Увеличиваем счетчик выпущенных паллет в заказе
  orderRecord.insert(
      "assembled_units",
      QString::number(orderRecord.value("assembled_units").toInt() + 1));
  if (!Database->updateRecordById("orders", orderRecord)) {
    emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                         "паллет в "
                         "заказе %1. ")
                     .arg(id));
    return false;
  }

  if (orderRecord.value("assembled_units").toInt() ==
      orderRecord.value("capacity").toInt()) {
    // Установка даты окончания и завершение процесса сборки заказа
    orderRecord.insert("ready_indicator", "true");
    orderRecord.insert("in_process", "false");
    orderRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                             TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("orders", orderRecord)) {
      emit logging(
          QString("Получена ошибка при установке даты начала сборки бокса %1. ")
              .arg(id));
      return false;
    }

    // Испускаем сигнал о конце сборки заказа
    emit orderAssemblingCompleted(&orderRecord);
  }

  return true;
}

bool TransponderReleaseSystem::searchNextTransponderForAssembling(
    QMap<QString, QString>* productionLineRecord) const {
  QMap<QString, QString> transponderRecord;
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> palletRecord;

  // Получаем данные о текущем транспондере
  transponderRecord.insert("id", productionLineRecord->value("transponder_id"));
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("box_id", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    emit logging(QString("Получена ошибка при поиске данных транспондера %1. ")
                     .arg(productionLineRecord->value("transponder_id")));
    return false;
  }

  // Получаем данные о текущем боксе
  boxRecord.insert("id", transponderRecord.value("box_id"));
  boxRecord.insert("pallet_id", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging(QString("Получена ошибка при поиске данных бокса %1. ")
                     .arg(transponderRecord.value("box_id")));
    return false;
  }

  // Получаем данные о текущей паллете
  palletRecord.insert("id", boxRecord.value("pallet_id"));
  palletRecord.insert("order_id", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    emit logging(QString("Получена ошибка при поиске данных паллеты %1. ")
                     .arg(boxRecord.value("pallet_id")));
    return false;
  }

  // Ищем в боксе следующий невыпущенный транспондер в текущем боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", "0");
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    emit logging(QString("Получена ошибка при поиске невыпущенного "
                         "транспондера в боксе %1. ")
                     .arg(transponderRecord.value("box_id")));
    return false;
  }

  // Если свободный транспондер в текущем боксе не найден
  if (transponderRecord.isEmpty()) {
    // Ищем свободный бокс в текущей паллете
    boxRecord.insert("id", "");
    boxRecord.insert("ready_indicator", "false");
    boxRecord.insert("in_process", "false");
    if (!Database->getRecordByPart("boxes", boxRecord)) {
      emit logging(
          QString("Получена ошибка при поиске свободного бокса в паллете %1. ")
              .arg(boxRecord.value("pallet_id")));
      return false;
    }

    // Если свободный бокс найден
    if (!boxRecord.isEmpty()) {
      // Запускаем сборку бокса
      boxRecord.insert("in_process", "true");
      boxRecord.insert(
          "assembling_start",
          QDateTime::currentDateTime().toString(TIMESTAMP_TEMPLATE));
      boxRecord.insert("production_line_id", productionLineRecord->value("id"));
      if (!Database->updateRecordById("boxes", boxRecord)) {
        emit logging(
            QString(
                "Получена ошибка при запуске сборки бокса %1 в паллете %2. ")
                .arg(boxRecord.value("id"), boxRecord.value("pallet_id")));
        return false;
      }

      // Ищем в запущенном боксе первый невыпущенный транспондер
      transponderRecord.insert("id", "");
      transponderRecord.insert("release_counter", "0");
      transponderRecord.insert("box_id", boxRecord.value("id"));
      if (!Database->getRecordByPart("transponders", transponderRecord)) {
        emit logging(QString("Получена ошибка при поиске невыпущенного "
                             "транспондера в боксе %1. ")
                         .arg(transponderRecord.value("box_id")));
        return false;
      }
    } else {  // Если свободных боксов в текущей паллете не найдено
      // Ищем свободную паллету в текущем заказе
      palletRecord.insert("id", "");
      palletRecord.insert("ready_indicator", "false");
      palletRecord.insert("in_process", "false");
      if (!Database->getRecordByPart("pallets", palletRecord)) {
        emit logging(
            QString(
                "Получена ошибка при поиске свободной паллеты в заказе %1. ")
                .arg(palletRecord.value("order_id")));
        return false;
      }

      // Если свободная паллета в текущем заказе найдена
      if (!palletRecord.isEmpty()) {
        // Запускаем сборку паллеты
        palletRecord.insert("in_process", "true");
        palletRecord.insert(
            "assembling_start",
            QDateTime::currentDateTime().toString(TIMESTAMP_TEMPLATE));
        if (!Database->updateRecordById("pallets", palletRecord)) {
          emit logging(
              QString(
                  "Получена ошибка при запуске сборки паллеты %1 в заказе %2. ")
                  .arg(palletRecord.value("id"),
                       palletRecord.value("order_id")));
          return false;
        }

        // Ищем первый бокс в найденной свободной паллете
        boxRecord.insert("id", "");
        boxRecord.insert("in_process", "false");
        boxRecord.insert("ready_indicator", "false");
        boxRecord.insert("pallet_id", palletRecord.value("id"));
        if (!Database->getRecordByPart("boxes", boxRecord)) {
          emit logging(QString("Получена ошибка при поиске невыпущенного "
                               "транспондера в боксе %1. ")
                           .arg(transponderRecord.value("box_id")));
          return false;
        }

        // Запускаем сборку бокса
        boxRecord.insert("in_process", "true");
        boxRecord.insert(
            "assembling_start",
            QDateTime::currentDateTime().toString(TIMESTAMP_TEMPLATE));
        boxRecord.insert("production_line_id",
                         productionLineRecord->value("id"));
        if (!Database->updateRecordById("boxes", boxRecord)) {
          emit logging(
              QString(
                  "Получена ошибка при запуске сборки бокса %1 в паллете %2. ")
                  .arg(boxRecord.value("id"), boxRecord.value("pallet_id")));
          return false;
        }

        // Ищем в запущенном боксе первый невыпущенный транспондер
        transponderRecord.insert("id", "");
        transponderRecord.insert("release_counter", "0");
        transponderRecord.insert("box_id", boxRecord.value("id"));
        if (!Database->getRecordByPart("transponders", transponderRecord)) {
          emit logging(QString("Получена ошибка при поиске невыпущенного "
                               "транспондера в боксе %1. ")
                           .arg(transponderRecord.value("box_id")));
          return false;
        }
      } else {  // Если свободной паллеты в текущем заказе не найдено
        emit logging(QString("В заказе %1 закончились свободные транспондеры. ")
                         .arg(palletRecord.value("order_id")));
        emit orderTranspondersOut();
        return true;
      }
    }
  }

  // Связываем линию производства с найденным транспондером
  productionLineRecord->insert("transponder_id", transponderRecord.value("id"));
  if (!Database->updateRecordById("production_lines", *productionLineRecord)) {
    emit logging(QString("Получена ошибка при связывании линии производства с "
                         "найденным транспондером %1. ")
                     .arg(productionLineRecord->value("transponder_id")));
    return false;
  }

  return true;
}

void TransponderReleaseSystem::proxyLogging(const QString& log) const {
  if (sender()->objectName() == "IDatabaseController") {
    emit logging("Database controller - " + log);
  } else {
    emit logging("Unknown - " + log);
  }
}

void TransponderReleaseSystem::on_OrderAssemblingCompleted_slot(
    const QMap<QString, QString>* orderData) {
  OrderAssembled = true;
}

void TransponderReleaseSystem::on_FreeTranspondersOut_slot() {
  FreeTranspondersOut = true;
}
