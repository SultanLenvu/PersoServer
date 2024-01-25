#include <QSettings>

#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(
    const QString& name,
    std::shared_ptr<AbstractSqlDatabase> db)
    : AbstractReleaseSystem(name, db) {
  loadSettings();
}

TransponderReleaseSystem::~TransponderReleaseSystem() {}

void TransponderReleaseSystem::setContext(
    std::shared_ptr<ProductionLineContext> context) {
  Context = context;
}

ReturnStatus TransponderReleaseSystem::release() {
  if (!Context->isInProcess()) {
    sendLog(QString("Производственная линия %1 не в процессе сборки.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotInProcess;
  }

  if (Context->transponder().get("box_id") !=
      Context->productionLine().get("box_id")) {
    sendLog(QString("Транспондер %1 не содержится в соответствующем боксе %1")
                .arg(Context->transponder().get("id"),
                     Context->productionLine().get("box_id")));
    return ReturnStatus::ConsistencyViolation;
  }

  if (Context->transponder().get("release_counter").toUInt() > 0) {
    sendLog(
        QString("Транспондер %1 уже был выпущен. Повторный выпуск невозможен.")
            .arg(Context->transponder().get("id")));
    return ReturnStatus::TransponderIncorrectRerelease;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::confirmRelease(const QString& ucid) {
  if (!Context->isInProcess()) {
    sendLog(QString("Производственная линия %1 не в процессе сборки.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotInProcess;
  }

  // Проверка того, что транспондер не был выпущен ранее
  if (Context->transponder().get("release_counter").toInt() >= 1) {
    sendLog(
        QString("Транспондер %1 был выпущен ранее. Подтверждение невозможно. ")
            .arg(Context->transponder().get("id")));
    return ReturnStatus::TransponderIncorrectRerelease;
  }

  // Проверка того, что транспондер ожидает подтверждения
  if (Context->transponder().get("awaiting_confirmation") == "false") {
    sendLog(QString("Транспондер %1 не был выпущен ранее.  Подтверждение "
                    "выпуска невозможно. ")
                .arg(Context->transponder().get("id")));
    return ReturnStatus::TransponderNotAwaitingConfirmation;
  }

  // Подтверждаем сборку транспондера
  if (!confirmCurrentTransponder(ucid)) {
    sendLog(QString("Получена ошибка при подтвеждении транспондера %1. ")
                .arg(Context->productionLine().get("transponder_id")));

    return ReturnStatus::DatabaseQueryError;
  }

  // Ищем новый транспондер для производственной линии
  ReturnStatus ret = searchNextTransponder();
  if (ret != ReturnStatus::NoError) {
    sendLog(QString("Получена ошибка при поиске очередного транспондера "
                    "для производственной линии %1. ")
                .arg(Context->productionLine().get("id")));
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
                .arg(Context->transponder().get("id")));
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
      QString::number(Context->transponder().get("release_counter").toInt() +
                      1));
  newTransponder.add("ucid", ucid);
  if (!Database->updateRecords("transponders",
                               QString("id = %1").arg(transponder.get("id")),
                               newTransponder)) {
    sendLog(QString("Получена ошибка при сохранении UCID транспондера %1. ")
                .arg(Context->transponder().get("id")));
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

ReturnStatus TransponderReleaseSystem::rollback() {
  if (!Context->isInProcess()) {
    sendLog(QString("Производственная линия %1 не в процессе сборки.")
                .arg(Context->login()));
    return ReturnStatus::ProductionLineNotInProcess;
  }

  SqlQueryValues newTransponder;
  SqlQueryValues transponder;
  SqlQueryValues newBox;
  SqlQueryValues newProductionLine;

  Database->setRecordMaxCount(1);
  Database->setCurrentOrder(Qt::DescendingOrder);

  if (Context->box().get("assembled_box") == "0") {
    sendLog(QString("Производственная линия '%1' связана с первым "
                    "транспондером в боксе. Откат невозможен.")
                .arg(Context->productionLine().get("id")));
    return ReturnStatus::ProductionLineRollbackLimit;
  }

  // Обновляем данные текущего транспондера
  newTransponder.add("release_counter", "0");
  newTransponder.add("ucid", "NULL");
  newTransponder.add("awaiting_confirmation", "false");
  if (!updateTransponder(newTransponder)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Уменьшаем количество собранных транспондеров в боксе
  newBox.add(
      "assembled_units",
      QString::number(Context->box().get("assembled_units").toInt() - 1));
  if (!updateBox(newBox)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Откатываем текущий транспондер
  QString prevTransponderId =
      QString::number(Context->transponder().get("id").toInt() - 1);
  if (!switchCurrentTransponder(prevTransponderId)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем производственную линию
  newProductionLine.add("transponder_id", Context->transponder().get("id"));
  if (!updateProductionLine(newProductionLine)) {
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
  if (!updateTransponder(newTransponder)) {
    return false;
  }

  // Увеличиваем счетчик выпущенных транспондеров в боксе
  newBox.add(
      "assembled_units",
      QString::number(Context->box().get("assembled_units").toInt() + 1));
  newBox.add("assembling_end", "NULL");
  if (!updateBox(newBox)) {
    return false;
  }

  // В противном случае возвращаемся
  return true;
}

ReturnStatus TransponderReleaseSystem::searchNextTransponder() {
  SqlQueryValues newProductionLine;
  QString nextTransponderId;

  // Если текущий бокс собран
  if (Context->box().get("assembled_units") == Context->box().get("quantity")) {
    sendLog(QString("Все транспондеры боксе %1 были собраны.")
                .arg(Context->box().get("id")));
    return ReturnStatus::BoxCompletelyAssembled;
  }

  // В противном случае двигаемся к следующему транспондеру в боксе
  nextTransponderId =
      QString::number(Context->transponder().get("id").toInt() + 1);

  // Переключаем текущий транспондер
  if (!switchCurrentTransponder(nextTransponderId)) {
    return ReturnStatus::DatabaseQueryError;
  }

  // Обновляем производственную линию
  newProductionLine.add("transponder_id", nextTransponderId);
  if (!updateProductionLine(newProductionLine)) {
    return ReturnStatus::DatabaseQueryError;
  }

  return ReturnStatus::NoError;
}

bool TransponderReleaseSystem::switchCurrentTransponder(const QString& id) {
  if (!Database->readRecords("transponders", QString("id = %1").arg(id),
                             Context->transponder())) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(id));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::switchCurrentBox(const QString& id) {
  if (!Database->readRecords("boxes", QString("id = %1").arg(id),
                             Context->box())) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ").arg(id));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::switchCurrentPallet(const QString& id) {
  if (!Database->readRecords("pallets", QString("id = %1").arg(id),
                             Context->pallet())) {
    sendLog(
        QString("Получена ошибка при получении данных паллеты %1. ").arg(id));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateProductionLine(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "production_lines",
          QString("id = %1").arg(Context->productionLine().get("id")),
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

bool TransponderReleaseSystem::updateTransponder(
    const SqlQueryValues& newValues) {
  if (!Database->updateRecords(
          "transponders",
          QString("id = %1").arg(Context->transponder().get("id")),
          newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных транспондера %1. ")
                .arg(Context->transponder().get("id")));
    return false;
  }

  if (!Database->readRecords(
          "transponders",
          QString("id = %1").arg(Context->transponder().get("id")),
          Context->transponder())) {
    sendLog(QString("Получена ошибка при получении данных транспондера %1. ")
                .arg(Context->transponder().get("id")));
    return false;
  }

  return true;
}

bool TransponderReleaseSystem::updateBox(const SqlQueryValues& newValues) {
  if (!Database->updateRecords("boxes",
                               QString("id = %1").arg(Context->box().get("id")),
                               newValues)) {
    sendLog(QString("Получена ошибка при обновлении данных бокса %1. ")
                .arg(Context->box().get("id")));
    return false;
  }

  if (!Database->readRecords("boxes",
                             QString("id = %1").arg(Context->box().get("id")),
                             Context->box())) {
    sendLog(QString("Получена ошибка при получении данных бокса %1. ")
                .arg(Context->box().get("id")));
    return false;
  }

  return true;
}
