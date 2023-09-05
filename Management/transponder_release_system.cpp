#include "transponder_release_system.h"

TransponderReleaseSystem::TransponderReleaseSystem(QObject* parent)
    : QObject(parent) {
  setObjectName("TransponderReleaseSystem");

  loadSettings();

  createDatabaseController();
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

bool TransponderReleaseSystem::release(TransponderInfoModel* seed) {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    emit logging("Получена ошибка при открытии транзакции. ");
    return false;
  }
  emit logging("Транзакция открыта. ");

  // Закрываем транзакцию
  if (Database->closeTransaction(IDatabaseController::Complete)) {
    emit logging("Транзакция закрыта. ");
    return true;
  } else {
    emit logging("Получена ошибка при закрытии транзакции. ");
    return false;
  }
}

bool TransponderReleaseSystem::confirmRelease(TransponderInfoModel* seed) {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    emit logging("Получена ошибка при открытии транзакции. ");
    return false;
  }
  emit logging("Транзакция открыта. ");

  // Закрываем транзакцию
  if (Database->closeTransaction(IDatabaseController::Complete)) {
    emit logging("Транзакция закрыта. ");
    return true;
  } else {
    emit logging("Получена ошибка при закрытии транзакции. ");
    return false;
  }
}

bool TransponderReleaseSystem::rerelease(TransponderInfoModel* seed) {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    emit logging("Получена ошибка при открытии транзакции. ");
    return false;
  }
  emit logging("Транзакция открыта. ");

  // Закрываем транзакцию
  if (Database->closeTransaction(IDatabaseController::Complete)) {
    emit logging("Транзакция закрыта. ");
    return true;
  } else {
    emit logging("Получена ошибка при закрытии транзакции. ");
    return false;
  }
}

bool TransponderReleaseSystem::search(TransponderInfoModel* seed) {
  QMap<QString, QString> transponderRecord;
  QMap<QString, QString>* mergedRecord = new QMap<QString, QString>;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    emit logging("Получена ошибка при открытии транзакции. ");
    return false;
  }

  tables.append("transponders");
  tables.append("boxes");
  tables.append("pallets");
  tables.append("orders");
  tables.append("issuers");
  foreignKeys.append("box_id");
  foreignKeys.append("pallet_id");
  foreignKeys.append("order_id");
  foreignKeys.append("issuer_id");
  mergedRecord->insert("transponders.id", "");
  mergedRecord->insert("model", "");
  mergedRecord->insert("release_counter", "");
  mergedRecord->insert("ucid", "");
  mergedRecord->insert("group_id", "");
  mergedRecord->insert("payment_means", "");
  mergedRecord->insert("efc_context_mark", "");
  mergedRecord->insert("boxes.in_process", "");
  mergedRecord->insert("transponders." + seed->getMap()->constBegin().key(),
                       seed->getMap()->constBegin().value());
  if (!Database->getMergedRecordByPart(tables, foreignKeys, *mergedRecord)) {
    emit logging("Ошибка при выполнении запроса. ");
  }
  emit logging("Запрос успешно выполнен. ");

  // Закрываем транзакцию
  if (!Database->closeTransaction(IDatabaseController::Complete)) {
    emit logging("Получена ошибка при закрытии транзакции. ");
    return false;
  }

  seed->build(mergedRecord);
  return true;
}

bool TransponderReleaseSystem::refund(TransponderInfoModel* seed) {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    emit logging("Получена ошибка при открытии транзакции. ");
    return false;
  }
  emit logging("Транзакция открыта. ");

  // Закрываем транзакцию
  if (Database->closeTransaction(IDatabaseController::Complete)) {
    emit logging("Транзакция закрыта. ");
    return true;
  } else {
    emit logging("Получена ошибка при закрытии транзакции. ");
    return false;
  }
}

void TransponderReleaseSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();

  Database->applySettings();
}

void TransponderReleaseSystem::createDatabaseController() {
  Database = new PostgresController(this, "ReleaserConnection");
  connect(Database, &IDatabaseController::logging, this,
          &TransponderReleaseSystem::proxyLogging);
}

void TransponderReleaseSystem::loadSettings() {}

void TransponderReleaseSystem::proxyLogging(const QString& log) const {
  if (sender()->objectName() == "IDatabaseController") {
    emit logging("Database controller - " + log);
  } else {
    emit logging("Unknown - " + log);
  }
}
