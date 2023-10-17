#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(QObject* parent)
    : QObject(parent) {
  setObjectName("TransponderReleaseSystem");
  loadSettings();

  // Создаем подключение к БД
  createDatabaseController();
}

void TransponderReleaseSystem::start(ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  sendLog("Запуск. ");
  if (!Database->connect()) {
    sendLog("Не удалось установить соединение с базой данных. ");
    *status = DatabaseConnectionError;
    return;
  }

  *status = Success;
}

void TransponderReleaseSystem::stop(void) {
  QMutexLocker locker(&Mutex);

  sendLog("Остановка. ");
  Database->disconnect();
}

void TransponderReleaseSystem::authorize(
    const QMap<QString, QString>* parameters,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QMap<QString, QString> productionLineRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  // Получаем данные о производственной линии
  productionLineRecord.insert("login", parameters->value("login"));
  productionLineRecord.insert("password", parameters->value("password"));
  productionLineRecord.insert("transponder_id", "");
  productionLineRecord.insert("active", "");
  if (!Database->getRecordByPart("production_lines", productionLineRecord)) {
    sendLog(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'. ")
            .arg(parameters->value("login")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  if (productionLineRecord.isEmpty()) {
    sendLog(QString("Производственная линия '%1' не найдена. ")
                .arg(parameters->value("login")));
    *status = ProductionLineMissed;
  } else if (productionLineRecord.value("active") == "false") {
    sendLog(QString("Производственная линия '%1' не активна. ")
                .arg(parameters->value("login")));
    *status = ProductionLineNotActive;
  } else {
    *status = Success;
  }

  emit operationFinished();
}

void TransponderReleaseSystem::release(
    const QMap<QString, QString>* releaseParameters,
    QMap<QString, QString>* attributes,
    QMap<QString, QString>* masterKeys,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QPair<QString, QString> searchPair;
  QMap<QString, QString> productionLineRecord;
  QMap<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  // Получаем данные о производственной линии
  productionLineRecord.insert("login", releaseParameters->value("login"));
  productionLineRecord.insert("password", releaseParameters->value("password"));
  productionLineRecord.insert("transponder_id", "");
  productionLineRecord.insert("id", "");
  productionLineRecord.insert("active", "");
  if (!Database->getRecordByPart("production_lines", productionLineRecord)) {
    sendLog(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'. ")
            .arg(releaseParameters->value("login")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Если производственная линия не активна, то возвращаемся
  if (productionLineRecord.value("active") == "false") {
    sendLog(QString("Производственная линия %1 не запущена. Выпуск "
                    "транспондера невозможен. ")
                .arg(productionLineRecord.value("id")));
    *status = ProductionLineNotActive;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Получаем данные о текущем транспондере
  transponderRecord.insert("id", productionLineRecord.value("transponder_id"));
  transponderRecord.insert("release_counter", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске данных транспондера '%1'. ")
                .arg(releaseParameters->value("login")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Если транспондер уже был выпущен, то повторный выпуск недопустим.
  // Перевыпуск должен осуществляться только соответствующей функцией
  if (transponderRecord.value("release_counter").toInt() != 0) {
    sendLog(
        QString("Транспондер %1 уже был выпущен, повторный выпуск невозможен. ")
            .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Ожидаем подтверждения
  transponderRecord.insert("id", productionLineRecord.value("transponder_id"));
  transponderRecord.insert("awaiting_confirmation", "true");
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при активации ожидания подтверждения "
                    "транспондера %1. ")
                .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Генерируем сид транспондера
  searchPair.first = "id";
  searchPair.second = transponderRecord.value("id");
  if (!getTransponderSeed(&searchPair, attributes, masterKeys)) {
    sendLog("Получена ошибка при генерации сида транспондера. ");
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  *status = Success;
  emit operationFinished();
}

void TransponderReleaseSystem::confirmRelease(
    const QMap<QString, QString>* confirmParameters,
    QMap<QString, QString>* transponderData,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QMap<QString, QString> productionLineRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = Failed;
    emit operationFinished();
    return;
  }

  // Запрашиваем данные о производственной линии
  productionLineRecord.insert("login", confirmParameters->value("login"));
  productionLineRecord.insert("password", confirmParameters->value("password"));
  productionLineRecord.insert("transponder_id", "");
  productionLineRecord.insert("id", "");
  productionLineRecord.insert("active", "");
  if (!Database->getRecordByPart("production_lines", productionLineRecord)) {
    sendLog("Получена ошибка при поиске данных производственной линии. ");
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Если производственная линия не активна, то возвращаемся
  if (productionLineRecord.value("active") == "false") {
    *status = ProductionLineNotActive;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Подтверждаем сборку транспондера
  if (!confirmTransponder(productionLineRecord.value("transponder_id"),
                          confirmParameters->value("ucid"))) {
    sendLog(QString("Получена ошибка при подтвеждении транспондера %1. ")
                .arg(productionLineRecord.value("transponder_id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Собираем информацию о транспондере
  if (!getTransponderData(productionLineRecord.value("transponder_id"),
                          transponderData)) {
    sendLog("Получена ошибка при сборе информации о выпущенном транспондере. ");
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Ищем новый транспондер для производственной линии
  if (!searchNextTransponderForAssembling(&productionLineRecord)) {
    sendLog(QString("Получена ошибка при поиске очередного транспондера "
                    "для производственной линии %1. ")
                .arg(productionLineRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = Failed;
    return;
  }
  *status = Success;
  emit operationFinished();
}

void TransponderReleaseSystem::rerelease(
    const QMap<QString, QString>* rereleaseParameters,
    QMap<QString, QString>* attributes,
    QMap<QString, QString>* masterKeys,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QPair<QString, QString> searchPair;
  QMap<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  // Получаем данные о перевыпускаемом транспондере
  transponderRecord.insert("id", rereleaseParameters->value("id"));
  transponderRecord.insert(
      "personal_account_number",
      rereleaseParameters->value("personal_account_number"));
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("ucid", "");
  transponderRecord.insert("box_id", "");
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске данных транспондера '%1'. ")
                .arg(rereleaseParameters->value("login")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Проверка, что транспондер найден
  if (transponderRecord.isEmpty()) {
    sendLog(QString("Транспондер не был найден, перевыпуск невозможен. "));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
  }

  // Проверка, что транспондер уже был выпущен ранее
  if (transponderRecord.value("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее"
                    "перевыпуск невозможен. ")
                .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
  }

  // Ожидаем подтверждения
  transponderRecord.insert("awaiting_confirmation", "true");
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при включении ожидания подтверждения "
                    "транспондера %1. ")
                .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Генерируем сид перевыпущенного транспондера
  searchPair.first = "id";
  searchPair.second = transponderRecord.value("id");
  if (!getTransponderSeed(&searchPair, attributes, masterKeys)) {
    sendLog("Получена ошибка при генерации сида транспондера. ");
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
  emit operationFinished();
}

void TransponderReleaseSystem::confirmRerelease(
    const QMap<QString, QString>* confirmParameters,
    QMap<QString, QString>* transponderData,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QMap<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  // Получаем данные о перевыпускаемом транспондере
  transponderRecord.insert("id", confirmParameters->value("id"));
  transponderRecord.insert("personal_account_number",
                           confirmParameters->value("personal_account_number"));
  transponderRecord.insert("ucid", "");
  transponderRecord.insert("release_counter", "");
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске данных транспондера '%1'. ")
                .arg(confirmParameters->value("login")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Осуществляем логические проверки
  if (!checkConfirmRerelease(transponderRecord, *confirmParameters)) {
    sendLog(QString("Получена логическая ошибка при подтверждении "
                    "перевыпуска транспондера %1. ")
                .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Собираем информацию о транспондере
  if (!getTransponderData(transponderRecord.value("id"), transponderData)) {
    sendLog(
        "Получена ошибка при сборе информации о перевыпущенном транспондере. ");
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Сохраняем UCID и увеличиваем счетчик выпусков
  transponderRecord.insert("awaiting_confirmation", "false");
  transponderRecord.insert(
      "release_counter",
      QString::number(transponderRecord.value("release_counter").toInt() + 1));
  transponderRecord.insert("ucid", confirmParameters->value("ucid"));
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при сохранении UCID транспондера %1. ")
                .arg(transponderRecord.value("id")));
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
  emit operationFinished();
}

void TransponderReleaseSystem::search(
    const QMap<QString, QString>* searchParameters,
    QMap<QString, QString>* attributes,
    QMap<QString, QString>* masterKeys,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QPair<QString, QString> searchPair;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = TransactionError;
    emit operationFinished();
    return;
  }

  searchPair.first = searchParameters->constBegin().key();
  searchPair.second = searchParameters->constBegin().value();
  if (!getTransponderSeed(&searchPair, attributes, masterKeys)) {
    sendLog("Получена ошибка при поиске транспондера. ");
    *status = Failed;
    Database->abortTransaction();
    emit operationFinished();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = TransactionError;
    return;
  }
  *status = Success;
  emit operationFinished();
}

void TransponderReleaseSystem::createDatabaseController() {
  Database = new PostgresController(this, "TransponderReleaseSystemConnection");
  connect(Database, &IDatabaseController::logging, LogSystem::instance(),
          &LogSystem::generate);
}

void TransponderReleaseSystem::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
}

void TransponderReleaseSystem::sendLog(const QString& log) const {
  if (LogEnable) {
    emit logging("TransponderReleaseSystem - " + log);
  }
}

bool TransponderReleaseSystem::checkConfirmRerelease(
    const QMap<QString, QString>& transponderRecord,
    const QMap<QString, QString>& searchData) {
  // Проверка, что транспондер найден
  if (transponderRecord.isEmpty()) {
    sendLog(QString(
        "Транспондер не был найден, подтверждение перевыпуска невозможен. "));
    return false;
  }

  // Проверка, что транспондер уже был выпущен ранее
  if (transponderRecord.value("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(transponderRecord.value("id")));
    return false;
  }

  // Проверка, что транспондер ожидает подтверждения
  if (transponderRecord.value("awaiting_confirmation") == "true") {
    sendLog(QString("Транспондер %1 еще не был перевыпущен, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(transponderRecord.value("id")));
    return false;
  }

  // Проверка, что новый UCID отличается от прошлого
  if (transponderRecord.value("ucid") == searchData.value("ucid")) {
    sendLog(QString("Новый UCID идентичен прошлому, повторный выпуск "
                    "транспондера %1 невозможен. ")
                .arg(transponderRecord.value("id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::confirmTransponder(const QString& transponderId,
                                                  const QString& ucid) const {
  QMap<QString, QString> transponderRecord;

  // Запрашиваем данные транспондера
  transponderRecord.insert("id", transponderId);
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("box_id", "");
  transponderRecord.insert("awaiting_confirmation", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске данных транспондера %1. ")
                .arg(transponderId));
    return false;
  }

  // Проверка того, что транспондер не был выпущен ранее
  if (transponderRecord.value("release_counter") != "0") {
    sendLog(
        QString("Транспондер %1 был выпущен ранее.  Подтверждение невозможно. ")
            .arg(transponderId));
    return false;
  }

  // Проверка того, что транспондер ожидает подтверждения
  if (transponderRecord.value("awaiting_confirmation") != "true") {
    sendLog(
        QString("Транспондер %1 не был выпущен.  Подтверждение невозможно. ")
            .arg(transponderId));
    return false;
  }

  // Увеличиваем счетчик выпусков транспондера и сохраняем ucid
  transponderRecord.insert("awaiting_confirmation", "false");
  transponderRecord.insert("ucid", ucid);
  transponderRecord.insert(
      "release_counter",
      QString::number(transponderRecord.value("release_counter").toInt() + 1));
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпусков "
                    "транспондера %1. ")
                .arg(transponderId));
    return false;
  }

  if (!confirmBox(transponderRecord.value("box_id"))) {
    sendLog(QString("Получена ошибка при подтверждении бокса %1. ")
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
  boxRecord.insert("quantity", "");
  boxRecord.insert("assembling_start", "");
  boxRecord.insert("assembling_end", "");
  boxRecord.insert("pallet_id", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    sendLog(QString("Получена ошибка при поиске данных бокса %1. ").arg(boxId));
    return false;
  }

  // Увеличиваем счетчик выпущенных транспондеров в боксе
  boxRecord.insert(
      "assembled_units",
      QString::number(boxRecord.value("assembled_units").toInt() + 1));
  if (!Database->updateRecordById("boxes", boxRecord)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "транспондеров в "
                    "боксе %1. ")
                .arg(boxId));
    return false;
  }

  // Если бокс целиком собран
  if (boxRecord.value("assembled_units").toInt() ==
      boxRecord.value("quantity").toInt()) {
    // Завершаем процесса сборки бокса
    boxRecord.insert("ready_indicator", "true");
    boxRecord.insert("in_process", "false");
    boxRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                           POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("boxes", boxRecord)) {
      sendLog(QString("Получена ошибка при завершении сборки бокса %1. ")
                  .arg(boxId));
      return false;
    }

    // Подтверждаем сборку в палете
    if (!confirmPallet(boxRecord.value("pallet_id"))) {
      sendLog(QString("Получена ошибка при подтверждении паллеты %1. ")
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
  palletRecord.insert("quantity", "");
  palletRecord.insert("assembling_start", "");
  palletRecord.insert("assembling_end", "");
  palletRecord.insert("order_id", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    sendLog(QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  // Увеличиваем счетчик выпущенных боксов в паллете
  palletRecord.insert(
      "assembled_units",
      QString::number(palletRecord.value("assembled_units").toInt() + 1));
  if (!Database->updateRecordById("pallets", palletRecord)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "боксов в "
                    "паллете %1. ")
                .arg(id));
    return false;
  }

  // Если паллета целиком собрана
  if (palletRecord.value("assembled_units").toInt() ==
      palletRecord.value("quantity").toInt()) {
    // Установка даты окончания и завершение процесса сборки паллеты
    palletRecord.insert("ready_indicator", "true");
    palletRecord.insert("in_process", "false");
    palletRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                              POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("pallets", palletRecord)) {
      sendLog(
          QString(
              "Получена ошибка при установке завершении сборки паллеты %1. ")
              .arg(id));
      return false;
    }

    // Подтверждаем сборку в заказе
    if (!confirmOrder(palletRecord.value("order_id"))) {
      sendLog(QString("Получена ошибка при подтверждении заказа %1. ")
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
  orderRecord.insert("quantity", "");
  orderRecord.insert("assembling_start", "");
  orderRecord.insert("assembling_end", "");
  if (!Database->getRecordById("orders", orderRecord)) {
    sendLog(QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  // Увеличиваем счетчик выпущенных паллет в заказе
  orderRecord.insert(
      "assembled_units",
      QString::number(orderRecord.value("assembled_units").toInt() + 1));
  if (!Database->updateRecordById("orders", orderRecord)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "паллет в "
                    "заказе %1. ")
                .arg(id));
    return false;
  }

  if (orderRecord.value("assembled_units").toInt() ==
      orderRecord.value("quantity").toInt()) {
    // Установка даты окончания и завершение процесса сборки заказа
    orderRecord.insert("ready_indicator", "true");
    orderRecord.insert("in_process", "false");
    orderRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                             POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("orders", orderRecord)) {
      sendLog(
          QString("Получена ошибка при завершении сборки заказа %1. ").arg(id));
      return false;
    }
  }

  return true;
}

bool TransponderReleaseSystem::searchNextTransponderForAssembling(
    QMap<QString, QString>* productionLineRecord) const {
  QMap<QString, QString> transponderRecord;
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> palletRecord;
  bool freeTranspondersRunOut = false;

  // Получаем данные о текущем транспондере
  transponderRecord.insert("id", productionLineRecord->value("transponder_id"));
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("box_id", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске данных транспондера %1. ")
                .arg(productionLineRecord->value("transponder_id")));
    return false;
  }

  // Получаем данные о текущем боксе
  boxRecord.insert("id", transponderRecord.value("box_id"));
  boxRecord.insert("pallet_id", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    sendLog(QString("Получена ошибка при поиске данных бокса %1. ")
                .arg(transponderRecord.value("box_id")));
    return false;
  }

  // Получаем данные о текущей паллете
  palletRecord.insert("id", boxRecord.value("pallet_id"));
  palletRecord.insert("order_id", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    sendLog(QString("Получена ошибка при поиске данных паллеты %1. ")
                .arg(boxRecord.value("pallet_id")));
    return false;
  }

  // Ищем в боксе следующий невыпущенный транспондер в текущем боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", "0");
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске невыпущенного "
                    "транспондера в боксе %1. ")
                .arg(transponderRecord.value("box_id")));
    return false;
  }

  // Если свободный транспондер в текущем боксе не найден
  if (transponderRecord.isEmpty()) {
    // Собираем данные о боксе и отправляем сигнал о завершении сборки бокса
    QSharedPointer<QMap<QString, QString> > boxData(
        new QMap<QString, QString>());
    if (!getBoxData(boxRecord.value("id"), boxData.get())) {
      sendLog(QString("Получена ошибка при получении данных бокса %1. ")
                  .arg(boxRecord.value("id")));
      return false;
    }
    emit boxAssemblingFinished(boxData);

    // Ищем свободный бокс в текущей паллете
    boxRecord.insert("id", "");
    boxRecord.insert("ready_indicator", "false");
    boxRecord.insert("in_process", "false");
    if (!Database->getRecordByPart("boxes", boxRecord)) {
      sendLog(
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
          QDateTime::currentDateTime().toString(POSTGRES_TIMESTAMP_TEMPLATE));
      boxRecord.insert("production_line_id", productionLineRecord->value("id"));
      if (!Database->updateRecordById("boxes", boxRecord)) {
        sendLog(
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
        sendLog(QString("Получена ошибка при поиске невыпущенного "
                        "транспондера в боксе %1. ")
                    .arg(transponderRecord.value("box_id")));
        return false;
      }
    } else {  // Если свободных боксов в текущей паллете не найдено
      // Собираем данные о паллете и отправляем сигнал о завершении сборки
      // паллеты
      QSharedPointer<QMap<QString, QString> > palletData(
          new QMap<QString, QString>());
      if (!getPalletData(palletRecord.value("id"), palletData.get())) {
        sendLog(QString("Получена ошибка при получении данных паллеты %1. ")
                    .arg(palletRecord.value("id")));
        return false;
      }
      emit palletAssemblingFinished(palletData);

      // Ищем свободную паллету в текущем заказе
      palletRecord.insert("id", "");
      palletRecord.insert("ready_indicator", "false");
      palletRecord.insert("in_process", "false");
      if (!Database->getRecordByPart("pallets", palletRecord)) {
        sendLog(
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
            QDateTime::currentDateTime().toString(POSTGRES_TIMESTAMP_TEMPLATE));
        if (!Database->updateRecordById("pallets", palletRecord)) {
          sendLog(
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
          sendLog(QString("Получена ошибка при поиске невыпущенного "
                          "транспондера в боксе %1. ")
                      .arg(transponderRecord.value("box_id")));
          return false;
        }

        // Запускаем сборку бокса
        boxRecord.insert("in_process", "true");
        boxRecord.insert(
            "assembling_start",
            QDateTime::currentDateTime().toString(POSTGRES_TIMESTAMP_TEMPLATE));
        boxRecord.insert("production_line_id",
                         productionLineRecord->value("id"));
        if (!Database->updateRecordById("boxes", boxRecord)) {
          sendLog(
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
          sendLog(QString("Получена ошибка при поиске невыпущенного "
                          "транспондера в боксе %1. ")
                      .arg(transponderRecord.value("box_id")));
          return false;
        }
      } else {  // Если свободной паллеты в текущем заказе не найдено
        sendLog(QString("В заказе %1 закончились свободные транспондеры. ")
                    .arg(palletRecord.value("order_id")));
        freeTranspondersRunOut = true;
      }
    }
  }

  if (freeTranspondersRunOut) {  // Если свободные транспондеры кончились, то
    // производственная линия останавливается
    productionLineRecord->insert("transponder_id", "NULL");
    productionLineRecord->insert("active", "false");
  } else {  // В противном случае связываем линию производства с найденным
            // транспондером
    productionLineRecord->insert("transponder_id",
                                 transponderRecord.value("id"));
  }
  if (!Database->updateRecordById("production_lines", *productionLineRecord)) {
    sendLog(QString("Получена ошибка при связывании линии производства с "
                    "найденным транспондером %1. ")
                .arg(productionLineRecord->value("transponder_id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::getTransponderSeed(
    const QPair<QString, QString>* searchPair,
    QMap<QString, QString>* attributes,
    QMap<QString, QString>* masterKeys) const {
  QStringList tables;
  QStringList foreignKeys;
  QString keyTableName;

  // Запрашиваем атрибуты
  tables.append("transponders");
  tables.append("boxes");
  tables.append("pallets");
  tables.append("orders");
  tables.append("issuers");
  foreignKeys.append("box_id");
  foreignKeys.append("pallet_id");
  foreignKeys.append("order_id");
  foreignKeys.append("issuer_id");

  attributes->insert("manufacturer_id", "");
  attributes->insert("equipment_class", "");
  attributes->insert("transponder_model", "");
  attributes->insert("accr_reference", "");
  attributes->insert("ucid", "");

  attributes->insert("efc_context_mark", "");
  attributes->insert("personal_account_number", "");

  attributes->insert("release_counter", "");
  attributes->insert("awaiting_confirmation", "");
  attributes->insert("full_personalization", "");
  attributes->insert("boxes.in_process", "");
  attributes->insert("transport_master_keys_id", "");
  attributes->insert("commercial_master_keys_id", "");
  attributes->insert("transponders." + searchPair->first, searchPair->second);

  if (!Database->getMergedRecordByPart(tables, foreignKeys, *attributes)) {
    return false;
  }

  // Генерируем дату активации батареи
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));
  attributes->insert("battery_insertation_date",
                     batteryInsertationDate.toUtf8());

  // Запрашиваем мастер ключи
  masterKeys->insert("accr_key", "");
  masterKeys->insert("per_key", "");
  masterKeys->insert("au_key1", "");
  masterKeys->insert("au_key2", "");
  masterKeys->insert("au_key3", "");
  masterKeys->insert("au_key4", "");
  masterKeys->insert("au_key5", "");
  masterKeys->insert("au_key6", "");
  masterKeys->insert("au_key7", "");
  masterKeys->insert("au_key8", "");
  if (attributes->value("full_personalization") == "false") {
    keyTableName = "transport_master_keys";
    masterKeys->insert("id", attributes->value("transport_master_keys_id"));
  } else {
    keyTableName = "commercial_master_keys";
    masterKeys->insert("id", attributes->value("commercial_master_keys_id"));
  }

  if (!Database->getRecordById(keyTableName, *masterKeys)) {
    return false;
  }
  masterKeys->remove("id");

  return true;
}

bool TransponderReleaseSystem::getTransponderData(
    const QString& id,
    QMap<QString, QString>* data) const {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Запрашиваем атрибуты
  tables.append("transponders");
  tables.append("boxes");
  tables.append("pallets");
  tables.append("orders");
  tables.append("issuers");
  foreignKeys.append("box_id");
  foreignKeys.append("pallet_id");
  foreignKeys.append("order_id");
  foreignKeys.append("issuer_id");

  mergedRecord.insert("manufacturer_id", "");
  mergedRecord.insert("equipment_class", "");
  mergedRecord.insert("transponder_model", "");
  mergedRecord.insert("accr_reference", "");
  mergedRecord.insert("ucid", "");

  mergedRecord.insert("efc_context_mark", "");
  mergedRecord.insert("personal_account_number", "");

  mergedRecord.insert("box_id", "");
  mergedRecord.insert("pallet_id", "");
  mergedRecord.insert("order_id", "");
  mergedRecord.insert("issuer_id", "");
  mergedRecord.insert("issuers.name", "");
  mergedRecord.insert("transponders.id", id);

  if (!Database->getMergedRecordByPart(tables, foreignKeys, mergedRecord)) {
    return false;
  }

  // Данные переносимые без изменений
  data->insert("box_id", mergedRecord.value("box_id"));
  data->insert("pallet_id", mergedRecord.value("pallet_id"));
  data->insert("order_id", mergedRecord.value("order_id"));
  data->insert("transponder_model", mergedRecord.value("transponder_model"));

  // Преобразуем в десятичный формат
  QString manufacturerId =
      QString::number(mergedRecord.value("manufacturer_id").toInt(nullptr, 16));

  // Дата сборки
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));

  // Дополняем серийник до 10 цифр нулями слева
  QString extendedTransponderId =
      QString("%1").arg(mergedRecord.value("id"), 10, QChar('0'));

  // Конструируем серийный номер транспондера
  data->insert("sn",
               QString("%1%2%3").arg(manufacturerId, batteryInsertationDate,
                                     extendedTransponderId));

  // Вычленяем символ F из personal_account_number
  QString tempPan = mergedRecord.value("personal_account_number");
  data->insert("pan", tempPan.remove(QChar('F')));

  // Название компании-заказчика
  data->insert("issuer_name", mergedRecord.value("name"));

  return true;
}

bool TransponderReleaseSystem::getBoxData(const QString& id,
                                          QMap<QString, QString>* data) const {
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> transponderRecord;
  QMap<QString, QString> transponderData;

  boxRecord.insert("id", id);
  boxRecord.insert("assembled_units", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging(QString("Получена ошибка при поиске бокса с id %1. ").arg(id));
    return false;
  }

  // Сохраняем данные бокса
  data->insert("id", id);
  data->insert("assembled_units", boxRecord.value("assembled_units"));

  // Ищем первый транспондер в боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("box_id", id);
  if (!Database->getRecordByPart("transponders", transponderRecord, true)) {
    emit logging(
        QString("Получена ошибка при поиске первого транспондера в боксе %1. ")
            .arg(id));
    return false;
  }

  // Запрашиваем данные транспондера
  if (!getTransponderData(transponderRecord.value("id"), &transponderData)) {
    emit logging(
        QString("Получена ошибка при получении данных транспондера %1. ")
            .arg(transponderRecord.value("id")));
    return false;
  }

  // Сохраняем серийник первого транспондера в боксе
  data->insert("first_transponder_sn", transponderData.value("sn"));
  transponderData.clear();
  transponderRecord.clear();

  // Ищем последний транспондер в боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("box_id", id);
  if (!Database->getRecordByPart("transponders", transponderRecord, false)) {
    emit logging(
        QString("Получена ошибка при поиске первого транспондера в боксе %1. ")
            .arg(id));
    return false;
  }

  // Запрашиваем данные транспондера
  if (!getTransponderData(transponderRecord.value("id"), &transponderData)) {
    emit logging(
        QString("Получена ошибка при получении данных транспондера %1. ")
            .arg(transponderRecord.value("id")));
    return false;
  }

  // Сохраняем серийник последнего транспондера в боксе
  data->insert("last_transponder_sn", transponderData.value("sn"));

  // Сохраняем модель транспондера
  QString modelTemp = transponderData.value("transponder_model");
  data->insert("transponder_model", modelTemp.remove(' '));

  // Добавляем полную дату сборки
  data->insert("assembling_date",
               QDate::currentDate().toString(BOX_STICKER_DATE_TEMPLATE));

  return true;
}

bool TransponderReleaseSystem::getPalletData(
    const QString& id,
    QMap<QString, QString>* data) const {
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> palletRecord;
  QMap<QString, QString> orderRecord;

  palletRecord.insert("id", id);
  palletRecord.insert("assembled_units", "");
  palletRecord.insert("order_id", "");
  palletRecord.insert("assembling_end", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    emit logging(
        QString("Получена ошибка при поиске паллеты с id %1. ").arg(id));
    return false;
  }

  orderRecord.insert("id", palletRecord.value("order_id"));
  orderRecord.insert("transponder_model", "");
  if (!Database->getRecordById("orders", orderRecord)) {
    emit logging(QString("Получена ошибка при поиске заказа с id %1. ")
                     .arg(palletRecord.value("order_id")));
    return false;
  }

  // Сохраняем данные паллеты
  data->insert("id", id);
  QStringList tempDate = palletRecord.value("assembling_end").split("T");
  data->insert(
      "assembly_date",
      QDate::fromString(tempDate.first(), "yyyy-MM-dd").toString("dd.MM.yyyy"));
  QString tempModel = orderRecord.value("transponder_model");
  data->insert("transponder_model", tempModel.remove(" "));

  // Ищем первый бокс в паллете
  boxRecord.insert("id", "");
  boxRecord.insert("pallet_id", id);
  if (!Database->getRecordByPart("boxes", boxRecord, true)) {
    emit logging(
        QString("Получена ошибка при поиске первого транспондера в боксе %1. ")
            .arg(id));
    return false;
  }

  // Сохраняем идентификатор первого бокса
  data->insert("first_box_id", boxRecord.value("id"));
  boxRecord.clear();

  // Ищем последний бокс в паллете
  boxRecord.insert("id", "");
  boxRecord.insert("quantity", "");
  boxRecord.insert("pallet_id", id);
  if (!Database->getRecordByPart("boxes", boxRecord, false)) {
    emit logging(
        QString("Получена ошибка при поиске первого транспондера в боксе %1. ")
            .arg(id));
    return false;
  }

  // Сохраняем идентификатор последнего бокса
  data->insert("last_box_id", boxRecord.value("id"));
  uint32_t totalQuantity = palletRecord.value("assembled_units").toInt() *
                           boxRecord.value("quantity").toInt();
  data->insert("quantity", QString::number(totalQuantity));

  return true;
}

bool TransponderReleaseSystem::getOrderData(
    const QString& id,
    QMap<QString, QString>* data) const {}
