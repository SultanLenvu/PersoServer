#include <QDateTime>

#include "production_line_launch_system.h"
#include "sql_query_values.h"

ProductionLineLaunchSystem::ProductionLineLaunchSystem(const QString& name)
    : AbstractLaunchSystem(name) {}

ProductionLineLaunchSystem::~ProductionLineLaunchSystem() {}

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

  SqlQueryValues newValues;
  newValues.add("launched", "true");
  if (!updateProductionLine(newValues)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::shutdown() {
  if (!SubContext->isLaunched()) {
    sendLog(QString("Производственная линия '%1' не была запущена. Остановка "
                    "не требуется.")
                .arg(SubContext->login()));
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

ReturnStatus ProductionLineLaunchSystem::checkProductionLineState() {
  if (SubContext->productionLine().get("active") == "false") {
    sendLog(
        QString(
            "Производственная линия '%1' не активирована. Запуск невозможен.")
            .arg(SubContext->login()));
    return ReturnStatus::ProductionLineNotActive;
  }

  if (SubContext->productionLine().get("launched") == "true") {
    sendLog(QString("Производственная линия '%1' уже запущена. Повторный "
                    "запуск невозможен.")
                .arg(SubContext->login()));
    return ReturnStatus::ProductionLineAlreadyLaunched;
  }

  if (SubContext->productionLine().get("in_process") == "true") {
    sendLog(QString("Производственная линия '%1' уже находится в процессе "
                    "работы. Повторный "
                    "запуск невозможен.")
                .arg(SubContext->login()));
    return ReturnStatus::ProductionLineAlreadyInProcess;
  }

  return ReturnStatus::NoError;
}

ReturnStatus ProductionLineLaunchSystem::loadProductionLine() {
  SubContext->productionLine().clear();
  Database->setRecordMaxCount(1);

  if (!Database->readRecords(
          "production_lines",
          QString("login = '%1' AND password = '%2'")
              .arg(SubContext->login(), SubContext->password()),
          SubContext->productionLine())) {
    sendLog(QString(
        "Получена ошибка при получении данных из таблицы production_lines."));
    return ReturnStatus::DatabaseQueryError;
  }

  if (SubContext->productionLine().isEmpty()) {
    sendLog(
        QString(
            "Получена ошибка при поиске данных производственной линии '%1'.")
            .arg(SubContext->login()));
    return ReturnStatus::ProductionLineMissed;
  }

  return ReturnStatus::NoError;
}

bool ProductionLineLaunchSystem::updateProductionLine(
    const SqlQueryValues& newValues) {
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
