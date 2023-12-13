#include "production_line_launch_system.h"
#include "Database/sql_query_values.h"
#include "General/definitions.h"
#include "Log/log_system.h"

ProductionLineLaunchSystem::ProductionLineLaunchSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractLaunchSystem(name, db) {
  loadSettings();
}

ProductionLineLaunchSystem::~ProductionLineLaunchSystem() {}

ReturnStatus ProductionLineLaunchSystem::init(
    const StringDictionary& param) const {
  sendLog("Инициализация всех производственных линий.");

  SqlQueryValues newValues;
  newValues.add("in_process", "false");
  if (!Database->updateRecords("production_lines", newValues)) {
    sendLog("Не удалось обновить записи в таблице production_lines.");
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::launch(
    const StringDictionary& param) const {
  sendLog(
      QString("Запуск производственной линии '%1'.").arg(param.value("login")));

  Database->setRecordMaxCount(1);

  SqlQueryValues pl;
  if (!Database->readRecords(
          "production_lines",
          QString("login = '%1' AND password = '%2'")
              .arg(param.value("login"), param.value("password")),
          pl)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (pl.isEmpty()) {
    sendLog(QString("Производственной линии '%1' не существует.")
                .arg(param.value("login")));
    return ReturnStatus::ProductionLineMissed;
  }

  if (pl.get("active") == "false") {
    sendLog(
        QString(
            "Производственная линия '%1' не активирована. Запуск невозможен.")
            .arg(param.value("login")));
    return ReturnStatus::ProductionLineNotActive;
  }

  if (pl.get("launched") == "true") {
    sendLog(QString("Производственная линия '%1' уже запущена. Повторный "
                    "запуск невозможен.")
                .arg(param.value("login")));
    return ReturnStatus::ProductionLineAlreadyLaunched;
  }

  if (pl.get("completed") == "true") {
    sendLog(QString("Производственная линия '%1' завершила свою работу. Запуск "
                    "невозможен.")
                .arg(param.value("login")));
    return ReturnStatus::ProductionLineCompleted;
  }

  ReturnStatus ret = attachWithFreeBox(pl.get("id"));
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось связать производственную линию '%1' с "
                    "свободным боксом.")
                .arg(param.value("login")));
    return ret;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::shutdown(
    const StringDictionary& param) const {
  sendLog(
      QString("Запуск производственной линии '%1'.").arg(param.value("login")));
  Database->setRecordMaxCount(1);

  SqlQueryValues pl;
  if (!Database->readRecords(
          "production_lines",
          QString("login = '%1' AND password = '%2'")
              .arg(param.value("login"), param.value("password")),
          pl)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (pl.isEmpty()) {
    sendLog(QString("Производственной линии '%1' не существует.")
                .arg(param.value("login")));
    return ReturnStatus::ProductionLineMissed;
  }

  ReturnStatus ret = detachFromBox(pl.get("id"), pl.get("box_id"));
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Не удалось отвязать производственную линию '%1' с "
                    "свободным боксом.")
                .arg(param.value("login")));
    return ret;
  }

  return ReturnStatus::NoError;
}

bool ProductionLineLaunchSystem::isLaunched(
    const StringDictionary& param) const {
  Database->setRecordMaxCount(1);

  SqlQueryValues pl;
  if (!Database->readRecords(
          "production_lines",
          QString("login = '%1' AND password = '%2'")
              .arg(param.value("login"), param.value("password")),
          pl)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return false;
  }

  if (pl.isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'.")
            .arg(param.value("login")));
    return false;
  }

  if (pl.get("launched") == "true") {
    return true;
  }

  return false;
}

void ProductionLineLaunchSystem::loadSettings() {}

void ProductionLineLaunchSystem::sendLog(const QString& log) const {
  LogSystem::instance()->generate(objectName() + " - " + log);
}

ReturnStatus ProductionLineLaunchSystem::attachWithFreeBox(
    const QString& id) const {
  SqlQueryValues nextTransponder;
  SqlQueryValues freeBox;
  SqlQueryValues boxNew;
  SqlQueryValues plNew;
  SqlQueryValues orderInProcess;
  QStringList tables{"boxes", "pallets", "orders"};

  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::AscendingOrder);

  if (!Database->readRecords("orders", QString("in_process = true"),
                             orderInProcess)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (orderInProcess.isEmpty()) {
    sendLog(QString("Не удалось найти заказ, находящийся в процессе сборки."));
    return ReturnStatus::OrderInProcessMissed;
  }

  if (!Database->readMergedRecords(
          tables,
          QString("((boxes.production_line_id = NULL OR "
                  "boxes.production_line_id = %1) AND orders.id = %2 AND "
                  "boxes.assembled_units < boxes.quantity)")
              .arg(id, orderInProcess.get("id")),
          freeBox)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (freeBox.isEmpty()) {
    sendLog(QString("Получена ошибка при поиске свободного бокса в заказе %1.")
                .arg(orderInProcess.get("id")));
    return ReturnStatus::FreeBoxMissed;
  }

  if (freeBox.get("assembled_units") == "0") {
    boxNew.add("production_line_id", id);
    boxNew.add("assembling_start", QDateTime::currentDateTime().toString(
                                       POSTGRES_TIMESTAMP_TEMPLATE));
  }
  boxNew.add("in_process", "true");
  if (!Database->updateRecords(
          "boxes", QString("id = %1").arg(freeBox.get("id")), boxNew)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (!Database->readRecords(
          "transponders",
          QString("release_counter = 0 AND box_id = %1").arg(freeBox.get("id")),
          nextTransponder)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (nextTransponder.isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске очередного транспондера в боксе %1.")
            .arg(freeBox.get("id")));
    return ReturnStatus::FreeBoxMissed;
  }

  plNew.add("box_id", freeBox.get("id"));
  plNew.add("transponder_id", nextTransponder.get("id"));
  plNew.add("launched", "true");
  if (!Database->updateRecords("production_lines", QString("id = %1").arg(id),
                               plNew)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::detachFromBox(
    const QString& id,
    const QString& boxId) const {
  SqlQueryValues attachedBox;
  SqlQueryValues boxNew;
  SqlQueryValues plNew;

  if (!Database->readRecords("boxes", QString("id = %1").arg(boxId),
                             attachedBox)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (attachedBox.get("assembled_units") == "0") {
    boxNew.add("production_line_id", "NULL");
    boxNew.add("assembling_start", "NULL");
  }
  boxNew.add("in_process", "false");
  boxNew.add("id", attachedBox.get("id"));
  if (!Database->updateRecords("boxes", QString("id = %1").arg(boxId),
                               boxNew)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  plNew.add("transponder_id", "NULL");
  plNew.add("box_id", "NULL");
  plNew.add("transponder_id", "NULL");
  plNew.add("launched", "false");
  if (!Database->updateRecords("production_lines", QString("id = %1").arg(id),
                               plNew)) {
    sendLog(QString("Получена ошибка при выполнении запроса в базу данных."));
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}
