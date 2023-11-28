#include "transponder_release_system.h"
#include "Database/postgre_sql_database.h"
#include "General/definitions.h"

TransponderReleaseSystem::TransponderReleaseSystem(QObject* parent)
    : QObject(parent) {
  setObjectName("TransponderReleaseSystem");
  loadSettings();

  qRegisterMetaType<TransponderReleaseSystem::ReturnStatus>(
      "TransponderReleaseSystem::ReturnStatus");
}

TransponderReleaseSystem::~TransponderReleaseSystem() {}

void TransponderReleaseSystem::instanceThreadStarted_slot() {
  // Создаем таймер проверки
  createCheckTimer();

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

  CheckTimer->start();
  *status = Completed;
}

void TransponderReleaseSystem::stop(void) {
  QMutexLocker locker(&Mutex);

  sendLog("Остановка. ");
  CheckTimer->stop();
  Database->disconnect();
}

void TransponderReleaseSystem::release(
    const QHash<QString, QString>* parameters,
    QHash<QString, QString>* seed,
    QHash<QString, QString>* data,
    TransponderReleaseSystem::ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Если транспондер уже был выпущен, то повторный выпуск недопустим.
  // Перевыпуск должен осуществляться только соответствующей функцией
  if (CurrentTransponder.value("release_counter").toInt() != 0) {
    sendLog(
        QString("Транспондер %1 уже был выпущен, повторный выпуск невозможен. ")
            .arg(CurrentTransponder.value("id")));
    *status = DatabaseQueryError;
    Database->rollbackTransaction();

    return;
  }

  // Ожидаем подтверждения
  CurrentTransponder.insert("awaiting_confirmation", "true");
  CurrentTransponder.insert("ucid", "NULL");
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при активации ожидания подтверждения "
                    "транспондера %1. ")
                .arg(CurrentTransponder.value("id")));
    *status = DatabaseQueryError;
    Database->rollbackTransaction();

    return;
  }

  // Генерируем сид транспондера
  generateFirmwareSeed(seed);

  // Собираем информацию о транспондере
  generateTransponderData(data);

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  *status = Completed;
}

void TransponderReleaseSystem::confirmRelease(
    const QHash<QString, QString>* parameters,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);
  QHash<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseQueryError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Проверка того, что транспондер не был выпущен ранее
  if (CurrentTransponder.value("release_counter").toInt() >= 1) {
    sendLog(
        QString("Транспондер %1 был выпущен ранее. Подтверждение невозможно. ")
            .arg(CurrentTransponder.value("id")));
    *status = TransponderNotReleasedEarlier;
    return;
  }

  // Проверка того, что транспондер ожидает подтверждения
  if (CurrentTransponder.value("awaiting_confirmation") != "true") {
    sendLog(
        QString("Транспондер %1 не был выпущен.  Подтверждение невозможно. ")
            .arg(CurrentTransponder.value("id")));
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
                .arg(CurrentTransponder.value("id")));
    *status = IdenticalUcidError;
    return;
  }

  // Подтверждаем сборку транспондера
  if (!confirmCurrentTransponder(parameters->value("ucid"))) {
    sendLog(QString("Получена ошибка при подтвеждении транспондера %1. ")
                .arg(CurrentProductionLine.value("transponder_id")));
    *status = DatabaseQueryError;
    Database->rollbackTransaction();

    return;
  }

  // Ищем новый транспондер для производственной линии
  if (!searchNextTransponderForCurrentProductionLine()) {
    sendLog(QString("Получена ошибка при поиске очередного транспондера "
                    "для производственной линии %1. ")
                .arg(CurrentProductionLine.value("id")));
    *status = DatabaseQueryError;
    Database->rollbackTransaction();

    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseQueryError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::rerelease(
    const QHash<QString, QString>* parameters,
    QHash<QString, QString>* seed,
    QHash<QString, QString>* data,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Проверка, что транспондер уже был выпущен ранее
  if (CurrentTransponder.value("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее"
                    "перевыпуск невозможен. ")
                .arg(CurrentTransponder.value("id")));
    *status = TransponderNotReleasedEarlier;
    Database->rollbackTransaction();
  }

  // Ожидаем подтверждения
  CurrentTransponder.insert("awaiting_confirmation", "true");
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при включении ожидания подтверждения "
                    "транспондера %1. ")
                .arg(CurrentTransponder.value("id")));
    *status = DatabaseQueryError;
    Database->rollbackTransaction();

    return;
  }

  // Генерируем сид транспондера
  generateFirmwareSeed(seed);

  // Собираем информацию о транспондере
  generateTransponderData(data);

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::confirmRerelease(
    const QHash<QString, QString>* parameters,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);
  QHash<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Проверка, что транспондер не был выпущен ранее
  if (CurrentTransponder.value("release_counter").toInt() <= 0) {
    sendLog(QString("Транспондер %1 еще не был выпущен ранее, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(CurrentTransponder.value("id")));
    *status = TransponderNotReleasedEarlier;
    return;
  }

  // Проверка, что транспондер ожидает подтверждения
  if (CurrentTransponder.value("awaiting_confirmation") != "true") {
    sendLog(QString("Транспондер %1 еще не был перевыпущен, подтверждение "
                    "перевыпуска невозможно. ")
                .arg(CurrentTransponder.value("id")));
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
                .arg(CurrentTransponder.value("id")));
    *status = IdenticalUcidError;
    return;
  }

  // Сохраняем UCID и увеличиваем счетчик выпусков
  CurrentTransponder.insert("awaiting_confirmation", "false");
  CurrentTransponder.insert(
      "release_counter",
      QString::number(CurrentTransponder.value("release_counter").toInt() + 1));
  CurrentTransponder.insert("ucid", parameters->value("ucid"));
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при сохранении UCID транспондера %1. ")
                .arg(CurrentTransponder.value("id")));
    *status = DatabaseQueryError;
    Database->rollbackTransaction();

    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::search(
    const QHash<QString, QString>* parameters,
    QHash<QString, QString>* data,
    TransponderReleaseSystem::ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Собираем информацию о транспондере
  generateTransponderData(data);

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::rollbackProductionLine(
    const QHash<QString, QString>* parameters,
    ReturnStatus* status) {
  QMutexLocker locker(&Mutex);

  QHash<QString, QString> transponderRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", ">0");
  transponderRecord.insert("box_id", CurrentBox.value("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord, false)) {
    sendLog(QString("Получена ошибка при поиске предыдущего транспондера "
                    "производственной линии в боксе %1. ")
                .arg(transponderRecord.value("box_id")));
    Database->rollbackTransaction();
    *status = DatabaseQueryError;

    return;
  }

  if (transponderRecord.isEmpty()) {
    sendLog(QString("Производственная линия '%1' связана с первым "
                    "транспондером в боксе. Откат невозможен.")
                .arg(CurrentProductionLine.value("id")));
    Database->rollbackTransaction();
    *status = ProductionLineRollbackLimitError;

    return;
  }

  transponderRecord.insert("release_counter", "0");
  transponderRecord.insert("ucid", "NULL");
  transponderRecord.insert("awaiting_confirmation", "false");
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при возврате транспондера %1.")
                .arg(transponderRecord.value("id")));
    Database->rollbackTransaction();
    *status = DatabaseQueryError;

    return;
  }

  CurrentBox.insert(
      "assembled_units",
      QString::number(CurrentBox.value("assembled_units").toInt() - 1));
  CurrentBox.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("boxes", CurrentBox)) {
    sendLog(QString("Получена ошибка при уменьшении количества собранных "
                    "транспондеров в боксе '%1'. ")
                .arg(transponderRecord.value("box_id")));
    Database->rollbackTransaction();
    *status = DatabaseQueryError;

    return;
  }

  CurrentProductionLine.insert("transponder_id", transponderRecord.value("id"));
  if (!Database->updateRecordById("production_lines", CurrentProductionLine)) {
    sendLog(QString("Получена ошибка при связывании производственной линии %1 "
                    "с транспондером %2. ")
                .arg(CurrentProductionLine.value("id"),
                     transponderRecord.value("id")));
    Database->rollbackTransaction();
    *status = DatabaseQueryError;

    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::getBoxData(
    const QHash<QString, QString>* parameters,
    QHash<QString, QString>* data,
    ReturnStatus* status) {
  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Получаем данные о боксе
  generateBoxData(data);

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::getPalletData(
    const QHash<QString, QString>* parameters,
    QHash<QString, QString>* data,
    ReturnStatus* status) {
  // Открываем транзакцию
  if (!Database->openTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }

  // Получаем текущий контекст
  *status = getCurrentContext(parameters);
  if (*status != Completed) {
    sendLog(QString("Получена ошибка при получении контекста производственной "
                    "линии '%1'. ")
                .arg(parameters->value("login")));
    Database->rollbackTransaction();

    return;
  }

  // Получаем данные о боксе
  generatePalletData(data);

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    *status = DatabaseTransactionError;

    return;
  }
  *status = Completed;
}

void TransponderReleaseSystem::createDatabaseController() {
  Database = new PostgreSqlDatabase(this, "TransponderReleaseSystemConnection");
  connect(Database, &IDatabaseController::logging, LogSystem::instance(),
          &LogSystem::generate);
}

void TransponderReleaseSystem::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
  CheckPeriod =
      settings.value("transponder_release_system/check_period").toUInt();
}

void TransponderReleaseSystem::sendLog(const QString& log) const {
  if (LogEnable) {
    emit const_cast<TransponderReleaseSystem*>(this)->logging(
        "TransponderReleaseSystem - " + log);
  }
}

void TransponderReleaseSystem::createCheckTimer() {
  CheckTimer = new QTimer(this);
  CheckTimer->setInterval(CheckPeriod * 1000);

  connect(CheckTimer, &QTimer::timeout, this,
          &TransponderReleaseSystem::on_CheckTimerTemeout);
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::getCurrentContext(
    const QHash<QString, QString>* initData) {
  QString masterKeyTableName = "transport_master_keys";
  QHash<QString, QString> transponderRecord;
  QHash<QString, QString> boxRecord;

  // Очищаем прошлый контекст
  clearCurrentContext();

  if (!initData->contains("personal_account_number") &&
      !initData->contains("id")) {
    CurrentProductionLine.insert("id", "");
    CurrentProductionLine.insert("login", initData->value("login"));
    CurrentProductionLine.insert("password", initData->value("password"));
    CurrentProductionLine.insert("active", "");
    CurrentProductionLine.insert("transponder_id", "");
    if (!Database->getRecordByPart("production_lines", CurrentProductionLine)) {
      sendLog(QString("Получена ошибка при получении данных производственной "
                      "линии '%1'. ")
                  .arg(initData->value("login")));
      return DatabaseQueryError;
    }

    if (CurrentProductionLine.isEmpty()) {
      sendLog(QString("Производственная линия '%1' не найдена. Сброс.")
                  .arg(initData->value("login")));
      return ProductionLineMissed;
    }

    if (CurrentProductionLine.value("active") == "false") {
      sendLog(QString("Производственная линия '%1' не активна. Сброс.")
                  .arg(initData->value("login")));
      return ProductionLineNotActive;
    }

    CurrentTransponder.insert("id",
                              CurrentProductionLine.value("transponder_id"));
    CurrentTransponder.insert("personal_account_number", "");
  } else {
    CurrentProductionLine.clear();
    CurrentTransponder.insert("id", initData->value("id"));
    CurrentTransponder.insert("personal_account_number",
                              initData->value("personal_account_number"));
  }
  CurrentTransponder.insert("release_counter", "");
  CurrentTransponder.insert("ucid", "");
  CurrentTransponder.insert("awaiting_confirmation", "");
  CurrentTransponder.insert("box_id", "");
  if (!Database->getRecordByPart("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(CurrentProductionLine.value("transponder_id")));
    return DatabaseQueryError;
  }

  if (CurrentTransponder.isEmpty()) {
    if (CurrentTransponder.contains("id")) {
      sendLog(QString("Транспондер %1 не найден. Сброс. ")
                  .arg(initData->value("id")));
    } else if (CurrentTransponder.contains("personal_account_number")) {
      sendLog(QString("Транспондер %1 не найден. Сброс. ")
                  .arg(initData->value("id")));
    } else {
      sendLog(QString("Транспондер не найден. Сброс. "));
    }
    return TransponderNotFound;
  }

  CurrentBox.insert("id", CurrentTransponder.value("box_id"));
  CurrentBox.insert("quantity", "");
  CurrentBox.insert("ready_indicator", "");
  CurrentBox.insert("in_process", "");
  CurrentBox.insert("assembled_units", "");
  CurrentBox.insert("assembling_start", "");
  CurrentBox.insert("assembling_end", "");
  CurrentBox.insert("pallet_id", "");
  CurrentBox.insert("production_line_id", "");
  if (!Database->getRecordById("boxes", CurrentBox)) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ")
                .arg(CurrentTransponder.value("box_id")));
    return DatabaseQueryError;
  }

  // Ищем первый транспондер в текущем боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("box_id", CurrentBox.value("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord, true)) {
    emit logging(
        QString("Получена ошибка при поиске первого транспондера в боксе %1. ")
            .arg(CurrentBox.value("id")));
    return DatabaseQueryError;
  }
  SupportData.insert("first_transponder_id", transponderRecord.value("id"));

  // Ищем последний транспондер в текущем боксе
  transponderRecord.insert("id", "");
  transponderRecord.insert("box_id", CurrentBox.value("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord, false)) {
    emit logging(
        QString(
            "Получена ошибка при поиске последнего транспондера в боксе %1. ")
            .arg(CurrentBox.value("id")));
    return DatabaseQueryError;
  }
  SupportData.insert("last_transponder_id", transponderRecord.value("id"));

  CurrentPallet.insert("id", CurrentBox.value("pallet_id"));
  CurrentPallet.insert("quantity", "");
  CurrentPallet.insert("ready_indicator", "");
  CurrentPallet.insert("in_process", "");
  CurrentPallet.insert("assembled_units", "");
  CurrentPallet.insert("assembling_start", "");
  CurrentPallet.insert("assembling_end", "");
  CurrentPallet.insert("shipped_indicator", "");
  CurrentPallet.insert("order_id", "");
  if (!Database->getRecordById("pallets", CurrentPallet)) {
    sendLog(QString("Получена ошибка при получении данных паллеты %1. ")
                .arg(CurrentBox.value("pallet_id")));
    return DatabaseQueryError;
  }

  // Ищем первый бокс в текущей паллете
  boxRecord.insert("id", "");
  boxRecord.insert("pallet_id", CurrentPallet.value("id"));
  if (!Database->getRecordByPart("boxes", boxRecord, true)) {
    emit logging(
        QString("Получена ошибка при поиске первого бокса в паллете %1. ")
            .arg(CurrentPallet.value("id")));
    return DatabaseQueryError;
  }
  SupportData.insert("first_box_id", boxRecord.value("id"));

  // Ищем последний бокс в текущей паллете
  boxRecord.insert("id", "");
  boxRecord.insert("pallet_id", CurrentPallet.value("id"));
  if (!Database->getRecordByPart("boxes", boxRecord, false)) {
    emit logging(
        QString("Получена ошибка при поиске последнего бокса в паллете %1. ")
            .arg(CurrentPallet.value("id")));
    return DatabaseQueryError;
  }
  SupportData.insert("last_box_id", boxRecord.value("id"));

  CurrentOrder.insert("id", CurrentPallet.value("order_id"));
  CurrentOrder.insert("quantity", "");
  CurrentOrder.insert("full_personalization", "");
  CurrentOrder.insert("ready_indicator", "");
  CurrentOrder.insert("in_process", "");
  CurrentOrder.insert("assembled_units", "");
  CurrentOrder.insert("assembling_start", "");
  CurrentOrder.insert("assembling_end", "");
  CurrentOrder.insert("transponder_model", "");
  CurrentOrder.insert("accr_reference", "");
  CurrentOrder.insert("manufacturer_id", "");
  CurrentOrder.insert("equipment_class", "");
  CurrentOrder.insert("shipped_units", "");
  CurrentOrder.insert("fully_shipped", "");
  CurrentOrder.insert("issuer_id", "");
  if (!Database->getRecordById("orders", CurrentOrder)) {
    sendLog(QString("Получена ошибка при получении данных заказа %1. ")
                .arg(CurrentPallet.value("order_id")));
    return DatabaseQueryError;
  }

  CurrentIssuer.insert("id", CurrentOrder.value("issuer_id"));
  CurrentIssuer.insert("name", "");
  CurrentIssuer.insert("efc_context_mark", "");
  CurrentIssuer.insert("transport_master_keys_id", "");
  CurrentIssuer.insert("commercial_master_keys_id", "");
  if (!Database->getRecordById("issuers", CurrentIssuer)) {
    sendLog(QString("Получена ошибка при получении данных заказчика %1. ")
                .arg(CurrentOrder.value("issuer_id")));
    return DatabaseQueryError;
  }

  // В зависимости от типа персонализации, берем те или иные мастер ключи
  if (CurrentOrder.value("full_personalization") == "true") {
    masterKeyTableName = "commercial_master_keys";
  }
  CurrentMasterKeys.insert("id",
                           CurrentIssuer.value(masterKeyTableName + "_id"));
  CurrentMasterKeys.insert("accr_key", "");
  CurrentMasterKeys.insert("per_key", "");
  CurrentMasterKeys.insert("au_key1", "");
  CurrentMasterKeys.insert("au_key2", "");
  CurrentMasterKeys.insert("au_key3", "");
  CurrentMasterKeys.insert("au_key4", "");
  CurrentMasterKeys.insert("au_key5", "");
  CurrentMasterKeys.insert("au_key6", "");
  CurrentMasterKeys.insert("au_key7", "");
  CurrentMasterKeys.insert("au_key8", "");
  if (!Database->getRecordById(masterKeyTableName, CurrentMasterKeys)) {
    sendLog(QString("Получена ошибка при получении мастер ключей %1. ")
                .arg(CurrentIssuer.value(masterKeyTableName + "_id")));
    return DatabaseQueryError;
  }

  return Completed;
}

void TransponderReleaseSystem::clearCurrentContext() {
  CurrentProductionLine.clear();
  CurrentTransponder.clear();
  CurrentBox.clear();
  CurrentPallet.clear();
  CurrentOrder.clear();
  CurrentIssuer.clear();
  CurrentMasterKeys.clear();
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::confirmCurrentTransponder(const QString& ucid) {
  // Увеличиваем счетчик выпусков транспондера и сохраняем ucid
  CurrentTransponder.insert("awaiting_confirmation", "false");
  CurrentTransponder.insert("ucid", ucid);
  CurrentTransponder.insert(
      "release_counter",
      QString::number(CurrentTransponder.value("release_counter").toInt() + 1));
  if (!Database->updateRecordById("transponders", CurrentTransponder)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпусков "
                    "транспондера %1. ")
                .arg(CurrentTransponder.value("id")));
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
      QString::number(CurrentBox.value("assembled_units").toInt() + 1));
  CurrentBox.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("boxes", CurrentBox)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "транспондеров в "
                    "боксе %1. ")
                .arg(CurrentBox.value("id")));
    return DatabaseQueryError;
  }

  // Если бокс целиком собран
  if (CurrentBox.value("assembled_units").toInt() ==
      CurrentBox.value("quantity").toInt()) {
    // Завершаем процесса сборки бокса
    CurrentBox.insert("ready_indicator", "true");
    CurrentBox.insert("in_process", "false");
    CurrentBox.insert("assembling_end", QDateTime::currentDateTime().toString(
                                            POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("boxes", CurrentBox)) {
      sendLog(QString("Получена ошибка при завершении сборки бокса %1. ")
                  .arg(CurrentBox.value("id")));
      return DatabaseQueryError;
    }

    // Собираем данные о боксе и отправляем сигнал о завершении сборки бокса
    QHash<QString, QString> boxData;
    IStickerPrinter::ReturnStatus status;
    generateBoxData(&boxData);
    emit boxAssemblingFinished(&boxData, &status);
    if (status != IStickerPrinter::Completed) {
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
      QString::number(CurrentPallet.value("assembled_units").toInt() + 1));
  CurrentPallet.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("pallets", CurrentPallet)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "боксов в "
                    "паллете %1. ")
                .arg(CurrentPallet.value("id")));
    return DatabaseQueryError;
  }

  // Если паллета целиком собрана
  if (CurrentPallet.value("assembled_units").toInt() ==
      CurrentPallet.value("quantity").toInt()) {
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
              .arg(CurrentPallet.value("id")));
      return DatabaseQueryError;
    }

    // Собираем данные о паллете и отправляем сигнал о завершении сборки
    // паллеты
    QHash<QString, QString> palletData;
    IStickerPrinter::ReturnStatus status;
    generatePalletData(&palletData);
    emit palletAssemblingFinished(&palletData, &status);
    if (status != IStickerPrinter::Completed) {
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
      QString::number(CurrentOrder.value("assembled_units").toInt() + 1));
  CurrentOrder.insert("assembling_end", "NULL");
  if (!Database->updateRecordById("orders", CurrentOrder)) {
    sendLog(QString("Получена ошибка при увеличении счетчика выпущенных "
                    "паллет в "
                    "заказе %1. ")
                .arg(CurrentOrder.value("id")));
    return DatabaseQueryError;
  }

  if (CurrentOrder.value("assembled_units").toInt() ==
      CurrentOrder.value("quantity").toInt()) {
    // Установка даты окончания и завершение процесса сборки заказа
    CurrentOrder.insert("ready_indicator", "true");
    CurrentOrder.insert("in_process", "false");
    CurrentOrder.insert("assembling_end", QDateTime::currentDateTime().toString(
                                              POSTGRES_TIMESTAMP_TEMPLATE));
    if (!Database->updateRecordById("orders", CurrentOrder)) {
      sendLog(QString("Получена ошибка при завершении сборки заказа %1. ")
                  .arg(CurrentOrder.value("id")));
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
  transponderRecord.insert("box_id", CurrentBox.value("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске невыпущенного "
                    "транспондера в боксе %1. ")
                .arg(transponderRecord.value("box_id")));
    return NextTransponderNotFound;
  }

  // Если свободный транспондер в текущем боксе найден
  if (!transponderRecord.isEmpty()) {
    // Связываем текущую линию производства с найденным транспондером
    return linkCurrentProductionLine(transponderRecord.value("id"));
  }

  sendLog(
      QString("В боксе %1 кончились свободные транспондеры. Поиск следующего "
              "бокса для сборки.  ")
          .arg(CurrentBox.value("id")));

  // В противном случае ищем свободный бокс в текущей паллете
  boxRecord.insert("id", "");
  boxRecord.insert("ready_indicator", "false");
  boxRecord.insert("in_process", "false");
  boxRecord.insert("pallet_id", CurrentPallet.value("id"));
  if (!Database->getRecordByPart("boxes", boxRecord)) {
    sendLog(
        QString("Получена ошибка при поиске свободного бокса в паллете %1. ")
            .arg(boxRecord.value("pallet_id")));
    return NextTransponderNotFound;
  }

  // Если свободный бокс найден
  if (!boxRecord.isEmpty()) {
    sendLog(QString("Запуск сборки бокса %1.  ").arg(boxRecord.value("id")));
    // Запускаем сборку бокса
    return startBoxAssembling(boxRecord.value("id"));
  }

  sendLog(QString("В паллете %1 кончились свободные боксы. Поиск следующей "
                  "паллеты для сборки.  ")
              .arg(CurrentPallet.value("id")));

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
  mergedRecord.insert("pallets.order_id", CurrentOrder.value("id"));

  if (!Database->getMergedRecordByPart(tables, foreignKeys, mergedRecord)) {
    sendLog(QString("Получена ошибка при поиске свободного бокса в заказе %1. ")
                .arg(CurrentOrder.value("id")));
    return NextTransponderNotFound;
  }

  //  // Если свободных боксов в текущей паллете не найдено
  //  // Ищем свободную паллету или паллету в процессе сборки в текущем заказе
  //  palletRecord.insert("id", "");
  //  palletRecord.insert("in_process", "false");
  //  palletRecord.insert("ready_indicator", "false");
  //  palletRecord.insert("order_id", CurrentOrder.value("id"));
  //  if (!Database->getRecordByPart("pallets", palletRecord)) {
  //    sendLog(
  //        QString("Получена ошибка при поиске свободной паллеты в заказе %1.
  //        ")
  //            .arg(palletRecord.value("order_id")));
  //    return NextTransponderNotFound;
  //  }

  //  // Если свободная паллета в текущем заказе найдена
  //  if (!palletRecord.isEmpty()) {
  //    sendLog(
  //        QString("Запуск сборки паллеты %1.
  //        ").arg(palletRecord.value("id")));
  //    // Запускаем сборку паллеты
  //    return startPalletAssembling(palletRecord.value("id"));
  //  }

  // Если свободный бокс найден
  if (!mergedRecord.isEmpty()) {
    // Ищем данные его паллеты
    palletRecord.insert("id", mergedRecord.value("pallet_id"));
    palletRecord.insert("in_process", "");
    if (!Database->getRecordById("pallets", palletRecord)) {
      sendLog(QString("Получена ошибка при поиске данных паллеты %1.")
                  .arg(mergedRecord.value("pallet_id")));
      return NextTransponderNotFound;
    }

    if (palletRecord.value("in_process") == "true") {
      return startBoxAssembling(mergedRecord.value("id"));
    } else {
      return startPalletAssembling(palletRecord.value("id"));
    }
  }

  // Если свободной паллеты в текущем заказе не найдено
  sendLog(QString("В заказе %1 закончились свободные паллеты. "
                  "Производственная линия %2 останавливается. ")
              .arg(palletRecord.value("order_id"),
                   CurrentProductionLine.value("id")));
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
  boxRecord.insert("production_line_id", CurrentProductionLine.value("id"));
  if (!Database->updateRecordById("boxes", boxRecord)) {
    sendLog(
        QString("Получена ошибка при запуске сборки бокса %1 в паллете %2. ")
            .arg(boxRecord.value("id"), boxRecord.value("pallet_id")));
    return StartBoxAssemblingError;
  }

  // Ищем в запущенном боксе первый невыпущенный транспондер
  transponderRecord.insert("id", "");
  transponderRecord.insert("release_counter", "0");
  transponderRecord.insert("box_id", boxRecord.value("id"));
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    sendLog(QString("Получена ошибка при поиске невыпущенного "
                    "транспондера в боксе %1. ")
                .arg(transponderRecord.value("box_id")));
    return NextTransponderNotFound;
  }

  // Связываем текущую линию производства с найденным транспондером
  return linkCurrentProductionLine(transponderRecord.value("id"));
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
            .arg(palletRecord.value("id"), palletRecord.value("order_id")));
    return StartPalletAssemblingError;
  }

  // Ищем первый бокс в найденной свободной паллете
  boxRecord.insert("id", "");
  boxRecord.insert("in_process", "false");
  boxRecord.insert("ready_indicator", "false");
  boxRecord.insert("pallet_id", palletRecord.value("id"));
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

  return startBoxAssembling(boxRecord.value("id"));
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::linkCurrentProductionLine(const QString& id) {
  CurrentProductionLine.insert("transponder_id", id);
  if (!Database->updateRecordById("production_lines", CurrentProductionLine)) {
    sendLog(QString("Получена ошибка при связывании линии производства с  "
                    "транспондером %1. ")
                .arg(CurrentProductionLine.value("transponder_id")));
    return DatabaseQueryError;
  }

  return Completed;
}

TransponderReleaseSystem::ReturnStatus
TransponderReleaseSystem::stopCurrentProductionLine() {
  CurrentProductionLine.insert("transponder_id", "NULL");
  CurrentProductionLine.insert("active", "false");
  if (!Database->updateRecordById("production_lines", CurrentProductionLine)) {
    sendLog(QString("Получена ошибка при остановке линии производства %1. ")
                .arg(CurrentProductionLine.value("id")));
    return DatabaseQueryError;
  }

  return Completed;
}

void TransponderReleaseSystem::generateFirmwareSeed(
    QHash<QString, QString>* seed) const {
  // DSRC атрибуты
  seed->insert("personal_account_number",
               CurrentTransponder.value("personal_account_number"));
  seed->insert("id", CurrentTransponder.value("id"));

  seed->insert("efc_context_mark", CurrentIssuer.value("efc_context_mark"));

  seed->insert("manufacturer_id", CurrentOrder.value("manufacturer_id"));
  seed->insert("equipment_class", CurrentOrder.value("equipment_class"));
  seed->insert("transponder_model", CurrentOrder.value("transponder_model"));
  seed->insert("accr_reference", CurrentOrder.value("accr_reference"));

  // Генерируем дату активации батареи
  QDate date = QDate::currentDate();
  QString batteryInsertationDate =
      QString("%1%2")
          .arg(QString::number(date.weekNumber()), 2, QChar('0'))
          .arg(QString::number(date.year() % 100), 2, QChar('0'));
  seed->insert("battery_insertation_date", batteryInsertationDate.toUtf8());

  // Мастер ключи безопасности
  seed->insert("accr_key", CurrentMasterKeys.value("accr_key"));
  seed->insert("per_key", CurrentMasterKeys.value("per_key"));
  seed->insert("au_key1", CurrentMasterKeys.value("au_key1"));
  seed->insert("au_key2", CurrentMasterKeys.value("au_key2"));
  seed->insert("au_key3", CurrentMasterKeys.value("au_key3"));
  seed->insert("au_key4", CurrentMasterKeys.value("au_key4"));
  seed->insert("au_key5", CurrentMasterKeys.value("au_key5"));
  seed->insert("au_key6", CurrentMasterKeys.value("au_key6"));
  seed->insert("au_key7", CurrentMasterKeys.value("au_key7"));
  seed->insert("au_key8", CurrentMasterKeys.value("au_key8"));
}

void TransponderReleaseSystem::generateTransponderData(
    QHash<QString, QString>* data) const {
  // Данные переносимые без изменений
  data->insert("box_id", CurrentBox.value("id"));
  data->insert("pallet_id", CurrentPallet.value("id"));
  data->insert("order_id", CurrentOrder.value("id"));

  // Удаляем пробелы из названия модели
  QString tempModel = CurrentOrder.value("transponder_model");
  data->insert("transponder_model", tempModel.remove(" "));

  // Конструируем серийный номер транспондера
  data->insert("sn",
               generateTransponderSerialNumber(CurrentTransponder.value("id")));

  // Вычленяем символы F из personal_account_number
  QString tempPan = CurrentTransponder.value("personal_account_number");
  data->insert("pan", tempPan.remove(QChar('F')));

  // Название компании-заказчика
  data->insert("issuer_name", CurrentIssuer.value("name"));
}

void TransponderReleaseSystem::generateBoxData(
    QHash<QString, QString>* data) const {
  // Идентификатор бокса
  data->insert("id", CurrentBox.value("id"));

  // Количество транспондеров в боксе
  data->insert("quantity", CurrentBox.value("assembled_units"));

  // Сохраняем серийник первого транспондера в боксе
  data->insert("first_transponder_sn",
               generateTransponderSerialNumber(
                   SupportData.value("first_transponder_id")));

  // Сохраняем серийник последнего транспондера в боксе
  data->insert("last_transponder_sn",
               generateTransponderSerialNumber(
                   SupportData.value("last_transponder_id")));

  // Сохраняем модель транспондера
  QString model = CurrentOrder.value("transponder_model");
  data->insert("transponder_model", model.remove(' '));
}

void TransponderReleaseSystem::generatePalletData(
    QHash<QString, QString>* data) const {
  // Идентификатор паллеты
  data->insert("id", CurrentPallet.value("id"));

  // Дата окончания сборки
  QStringList tempDate = CurrentPallet.value("assembling_end").split("T");
  data->insert(
      "assembly_date",
      QDate::fromString(tempDate.first(), "yyyy-MM-dd").toString("dd.MM.yyyy"));

  // Модель транспондеров в паллете
  QString tempModel = CurrentOrder.value("transponder_model");
  data->insert("transponder_model", tempModel.remove(" "));

  // Сохраняем идентификатор первого бокса
  data->insert("first_box_id", SupportData.value("first_box_id"));

  // Сохраняем идентификатор последнего бокса
  data->insert("last_box_id", SupportData.value("last_box_id"));

  // Общее количество транспондеров в паллете
  uint32_t totalQuantity = CurrentPallet.value("assembled_units").toInt() *
                           CurrentBox.value("quantity").toInt();
  data->insert("quantity", QString::number(totalQuantity));
}

QString TransponderReleaseSystem::generateTransponderSerialNumber(
    const QString& id) const {
  // Преобразуем в десятичный формат
  QString manufacturerId =
      QString::number(CurrentOrder.value("manufacturer_id").toInt(nullptr, 16));

  // Дата сборки
  QStringList tempDate = CurrentBox.value("assembling_start").split("T");
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

void TransponderReleaseSystem::on_CheckTimerTemeout() {
  if (!Database->checkConnection()) {
    sendLog("Потеряно соединение с базой данных.");
    CheckTimer->stop();
    emit failed(DatabaseConnectionError);
  }
}
