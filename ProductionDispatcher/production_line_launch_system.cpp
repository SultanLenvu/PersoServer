#include <QDateTime>

#include "production_line_launch_system.h"
#include "sql_query_values.h"

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

  return ReturnStatus::NoError;
}

void ProductionLineLaunchSystem::loadSettings() {}

void ProductionLineLaunchSystem::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
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
