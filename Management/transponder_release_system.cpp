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

void TransponderReleaseSystem::release(const QMap<QString, QString>* searchData,
                                       QMap<QString, QString>* resultData,
                                       bool& ok) {
  QMap<QString, QString> productionLineRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    ok = false;
    return;
  }

  productionLineRecord.insert("login", searchData->value("login"));
  productionLineRecord.insert("password", searchData->value("password"));
  if (!Database->getRecordById("production_lines", productionLineRecord)) {
    emit logging("Получена ошибка при поиске данных производственной линии. ");
    ok = false;
    Database->abortTransaction();
    return;
  }

  if (!getTranponderData("id", productionLineRecord.value("transponder_id"),
                         resultData)) {
    emit logging("Получена ошибка при поиске транспондера. ");
    ok = false;
    Database->abortTransaction();
    return;
  }

  // Если транспондер уже был выпущен, то повторный выпуск недопустим.
  // Перевыпуск должен осуществляться только соответствующей функцией
  if (resultData->value("release_counter").toInt() != 0) {
    emit logging(
        "Транспондер уже был выпущен, повторный выпуск невозможен. Сброс. ");
    ok = false;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    ok = false;
    return;
  }
  ok = true;
}

void TransponderReleaseSystem::confirm(const QMap<QString, QString>* searchData,
                                       bool& ok) {
  QMap<QString, QString> productionLineRecord;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    ok = false;
    return;
  }

  // Запрашиваем данные о производственной линии
  productionLineRecord.insert("login", searchData->value("login"));
  productionLineRecord.insert("password", searchData->value("password"));
  if (!Database->getRecordById("production_lines", productionLineRecord)) {
    emit logging("Получена ошибка при поиске данных производственной линии. ");
    ok = false;
    Database->abortTransaction();
    return;
  }

  // Подтверждаем сборку транспондера
  if (!confirmTransponder(productionLineRecord.value("transponder_id"))) {
    emit logging(QString("Получена ошибка при подтвеждении транспондера. ")
                     .arg(productionLineRecord.value("transponder_id")));
    ok = false;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    ok = false;
    return;
  }
  ok = true;
}

void TransponderReleaseSystem::rerelease(
    const QMap<QString, QString>* searchData,
    QMap<QString, QString>* resultData,
    bool& ok) {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    ok = false;
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    ok = false;
    return;
  }
  ok = true;
}

void TransponderReleaseSystem::refund(const QMap<QString, QString>* searchData,
                                      bool& ok) {
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Открываем транзакцию
  if (!Database->openTransaction()) {
    ok = false;
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    ok = false;
    return;
  }
  ok = true;
}

void TransponderReleaseSystem::search(const QMap<QString, QString>* searchData,
                                      QMap<QString, QString>* resultData,
                                      bool& ok) {
  // Открываем транзакцию
  if (!Database->openTransaction()) {
    ok = false;
    return;
  }

  if (!getTranponderData(searchData->constBegin().key(),
                         searchData->constBegin().value(), resultData)) {
    emit logging("Получена ошибка при поиске транспондера. ");
    ok = false;
    Database->abortTransaction();
    return;
  }

  // Закрываем транзакцию
  if (!Database->closeTransaction()) {
    ok = false;
    return;
  }
  ok = true;
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
  resultData->insert("ucid", "");
  resultData->insert("group_id", "");
  resultData->insert("payment_means", "");
  resultData->insert("efc_context_mark", "");
  resultData->insert("boxes.in_process", "");
  resultData->insert("transponders." + searchKey, searchValue);

  if (!Database->getMergedRecordById(tables, foreignKeys, *resultData)) {
    return false;
  }
  return true;
}

bool TransponderReleaseSystem::confirmTransponder(const QString& id) const {
  QMap<QString, QString> transponderRecord;

  // Запрашиваем данные транспондера
  transponderRecord.insert("id", id);
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("box_id", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных транспондера %1. ").arg(id));
    return false;
  }

  // Увеличиваем счетчик выпусков транспондера
  transponderRecord.insert(
      "release_counter",
      QString::number(transponderRecord.value("release_counter").toInt() + 1));
  if (!Database->updateRecordById("transponders", transponderRecord)) {
    emit logging(QString("Получена ошибка при увеличении счетчика выпусков "
                         "транспондера %1. ")
                     .arg(id));
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

bool TransponderReleaseSystem::confirmBox(const QString& id) const {
  QMap<QString, QString> boxRecord;

  // Получаем данные о боксе
  boxRecord.insert("id", id);
  boxRecord.insert("assembled_units", "");
  boxRecord.insert("capacity", "");
  boxRecord.insert("assembling_start", "");
  boxRecord.insert("assembling_end", "");
  boxRecord.insert("pallet_id", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  if (boxRecord.value("assembled_units").toInt() == 0) {
    // Увеличиваем счетчик выпущенных транспондеров в боксе
    boxRecord.insert(
        "assembled_units",
        QString::number(boxRecord.value("assembled_units").toInt() + 1));
    if (!Database->updateRecordById("boxes", boxRecord)) {
      emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                           "транспондеров в "
                           "боксе %1. ")
                       .arg(id));
      return false;
    }

    // Установка даты начала бокса и запуск процесса сборки
    boxRecord.insert("in_process", "true");
    boxRecord.insert("assembling_start", QDateTime::currentDateTime().toString(
                                             "dd.MM.yyyy hh.mm.ss"));
    if (!Database->updateRecordById("boxes", boxRecord)) {
      emit logging(
          QString("Получена ошибка при установке даты начала сборки бокса %1. ")
              .arg(id));
      return false;
    }
  } else if (boxRecord.value("assembled_units") ==
             boxRecord.value("capacity")) {
    // Установка даты окончания и завершаем процесс сборки бокса
    boxRecord.insert("in_process", "false");
    boxRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                           "dd.MM.yyyy hh.mm.ss"));
    if (!Database->updateRecordById("boxes", boxRecord)) {
      emit logging(
          QString("Получена ошибка при установке даты начала сборки бокса %1. ")
              .arg(id));
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
  } else {
    // Увеличиваем счетчик выпущенных транспондеров в боксе
    boxRecord.insert(
        "assembled_units",
        QString::number(boxRecord.value("assembled_units").toInt() + 1));
    if (!Database->updateRecordById("boxes", boxRecord)) {
      emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                           "транспондеров в "
                           "боксе %1. ")
                       .arg(id));
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

  if (palletRecord.value("assembled_units").toInt() == 0) {
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

    // Установка даты начала сборки паллеты и запуск процесса
    palletRecord.insert("in_process", "true");
    palletRecord.insert(
        "assembling_start",
        QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"));
    if (!Database->updateRecordById("pallets", palletRecord)) {
      emit logging(
          QString("Получена ошибка при установке даты начала сборки бокса %1. ")
              .arg(id));
      return false;
    }
  } else if (palletRecord.value("assembled_units") ==
             palletRecord.value("capacity")) {
    // Установка даты окончания и завершение процесса сборки паллеты
    palletRecord.insert("in_process", "false");
    palletRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                              "dd.MM.yyyy hh.mm.ss"));
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
  } else {
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
  if (!Database->getRecordById("pallets", orderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  if (orderRecord.value("assembled_units").toInt() == 0) {
    // Увеличиваем счетчик выпущенных паллет в заказе
    orderRecord.insert(
        "assembled_units",
        QString::number(orderRecord.value("assembled_units").toInt() + 1));
    if (!Database->updateRecordById("pallets", orderRecord)) {
      emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                           "паллет в "
                           "заказе %1. ")
                       .arg(id));
      return false;
    }

    // Установка даты начала и запуск процесса сборки заказа
    orderRecord.insert("in_process", "true");
    orderRecord.insert(
        "assembling_start",
        QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"));
    if (!Database->updateRecordById("orders", orderRecord)) {
      emit logging(
          QString(
              "Получена ошибка при установке даты начала сборки заказа %1. ")
              .arg(id));
      return false;
    }
  } else if (orderRecord.value("assembled_units") ==
             orderRecord.value("capacity")) {
    // Установка даты окончания и завершение процесса сборки заказа
    orderRecord.insert("in_process", "false");
    orderRecord.insert("assembling_end", QDateTime::currentDateTime().toString(
                                             "dd.MM.yyyy hh.mm.ss"));
    if (!Database->updateRecordById("orders", orderRecord)) {
      emit logging(
          QString("Получена ошибка при установке даты начала сборки бокса %1. ")
              .arg(id));
      return false;
    }

    // Испускаем сигнал о конце сборки заказа
    emit orderAssemblingCompleted(&orderRecord);
  } else {
    // Увеличиваем счетчик выпущенных паллет в заказе
    orderRecord.insert(
        "assembled_units",
        QString::number(orderRecord.value("assembled_units").toInt() + 1));
    if (!Database->updateRecordById("pallets", orderRecord)) {
      emit logging(QString("Получена ошибка при увеличении счетчика выпущенных "
                           "паллет в "
                           "заказе %1. ")
                       .arg(id));
      return false;
    }
  }

  return true;
}

bool TransponderReleaseSystem::searchNextTransponderForAssembling(
    const QString& id) const {
  QMap<QString, QString> transponderRecord;
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> palletRecord;

  // Запрашиваем данные транспондера
  transponderRecord.insert("id", id);
  transponderRecord.insert("release_counter", "");
  transponderRecord.insert("box_id", "");
  if (!Database->getRecordById("transponders", transponderRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных транспондера %1. ").arg(id));
    return false;
  }

  // Получаем данные о боксе
  boxRecord.insert("id", transponderRecord.value("box_id"));
  boxRecord.insert("assembled_units", "");
  boxRecord.insert("capacity", "");
  boxRecord.insert("pallet_id", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }

  // Если в бокс еще не заполнен
  if (boxRecord.value("capacity") == boxRecord.value("assembled_units")) {
    // Ищем в боксе следующий невыпущенный транспондер
    transponderRecord.insert("id", "");
    transponderRecord.insert("transponders.release_counter", "0");
    if (!Database->getRecordByPart("transponders", transponderRecord)) {
      emit logging(QString("Получена ошибка при поиске невыпущенного "
                           "транспондера в боксе %1. ")
                       .arg(transponderRecord.value("box_id")));
      return false;
    }

    // Обновляем линию производства
  }

  // В противном случае
  // Ищем следующий свободный бокс в текущей паллете

  // Получаем данные о паллете
  palletRecord.insert("id", id);
  palletRecord.insert("assembled_units", "");
  palletRecord.insert("capacity", "");
  palletRecord.insert("order_id", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    emit logging(
        QString("Получена ошибка при поиске данных бокса %1. ").arg(id));
    return false;
  }
}

bool TransponderReleaseSystem::refundTransponder(const QString& id) const {}

bool TransponderReleaseSystem::refundBox(const QString& id) const {}

bool TransponderReleaseSystem::refundPallet(const QString& id) const {}

bool TransponderReleaseSystem::refundOrder(const QString& id) const {}

void TransponderReleaseSystem::proxyLogging(const QString& log) const {
  if (sender()->objectName() == "IDatabaseController") {
    emit logging("Database controller - " + log);
  } else {
    emit logging("Unknown - " + log);
  }
}
