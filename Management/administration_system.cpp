#include "administration_system.h"

AdministrationSystem::AdministrationSystem(QObject* parent) : QObject(parent) {
  setObjectName("AdministrationSystem");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");

  loadSettings();

  createDatabaseController();
}

void AdministrationSystem::proxyLogging(const QString& log) {
  if (sender()->objectName() == "IDatabaseController") {
    emit logging("Database controller - " + log);
  } else {
    emit logging("Unknown - " + log);
  }
}

void AdministrationSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
  Database->applySettings();
}

void AdministrationSystem::createDatabaseController() {
  // Создаем контроллер базы данных
  Database = new PostgresController(this, "AdministratorConnection");
  connect(Database, &IDatabaseController::logging, this,
          &AdministrationSystem::proxyLogging);
}

void AdministrationSystem::clearDatabaseTable(const QString& tableName) {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Очистка данных таблицы базы данных. ");
  if (Database->clearTable(tableName)) {
    processingResult("Очистка выполнена. ", CompletedSuccessfully);
  } else {
    processingResult("Очистка не выполнена. ", DatabaseQueryError);
  }
}

void AdministrationSystem::getDatabaseTable(const QString& tableName,
                                            DatabaseTableModel* buffer) {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Получение таблицы базы данных. ");
  Database->getTable(tableName, 10, buffer);

  if (buffer->isEmpty()) {
    processingResult("Ошибка при получении данных из таблицы базы данных. ",
                     DatabaseQueryError);
  } else {
    processingResult("Данные из таблицы базы данных получены. ",
                     CompletedSuccessfully);
  }
}

void AdministrationSystem::getCustomResponse(const QString& req,
                                             DatabaseTableModel* buffer) {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Выполнение кастомного запроса. ");
  if (!Database->execCustomRequest(req, buffer)) {
    processingResult("Ошибка при выполнении кастомного запроса. ",
                     DatabaseQueryError);
  } else {
    processingResult("Кастомный запрос успешно выполнен. ",
                     CompletedSuccessfully);
  }
}

void AdministrationSystem::initIssuerTable() {
  QMap<QString, QString> record;
  int32_t lastId = 0;

  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  // Получаем идентифкатор последнего добавленного заказа
  if (!Database->getLastRecord("issuers", record)) {
    processingResult("Ошибка при поиске последнего заказа. ",
                     DatabaseQueryError);
    return;
  }
  lastId = record.value("id").toInt();

  emit logging("Инициализация таблицы эмитентов. ");
  record.insert("id", QString::number(++lastId));
  record.insert("name", "Пауэр Синтез");
  record.insert("efc_context_mark", "000000000001");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("id", QString::number(++lastId));
  record.insert("name", "Автодор");
  record.insert("efc_context_mark", "570002FF0070");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("id", QString::number(++lastId));
  record.insert("name", "Новое качество дорог");
  record.insert("efc_context_mark", "000000000001");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("id", QString::number(++lastId));
  record.insert("name", "Западный скоростной диаметр");
  record.insert("efc_context_mark", "570001FF0070");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("id", QString::number(++lastId));
  record.insert("name", "Объединенные системы сбора платы");
  record.insert("efc_context_mark", "000000000001");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  processingResult("Инициализация успешно завершена. ", CompletedSuccessfully);
}

void AdministrationSystem::createNewOrder(
    const QMap<QString, QString>* orderParameters) {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Добавление заказа. ");
  if (!addOrder(orderParameters)) {
    processingResult("Получена ошибка при добавлении заказа. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление палет. ");
  if (!addPallets(orderParameters)) {
    processingResult("Получена ошибка при добавлении палет. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление боксов. ");
  if (!addBoxes(orderParameters)) {
    processingResult("Получена ошибка при добавлении боксов. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление транспондеров. ");
  if (!addTransponders(orderParameters)) {
    processingResult("Получена ошибка при добавлении транспондеров. ",
                     DatabaseQueryError);
    return;
  }

  processingResult("Новый заказ успешно создан. ", CompletedSuccessfully);
}

void AdministrationSystem::deleteLastOrder() {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Удаление последнего добавленного заказа. ");
  Database->removeLastRecordWithCondition(
      "orders", "(in_process = 'false' AND ready_indicator = 'false')");

  processingResult("Последний добавленный заказ успешно удален. ",
                   CompletedSuccessfully);
}

void AdministrationSystem::createNewProductionLine(
    const QMap<QString, QString>* productionLineParameters) {

  emit logging("Создание новой линии производства. ");

  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  // Добавляем линию производства
  emit logging("Добавление линии производства. ");
  if (!addProductionLine(productionLineParameters)) {
    processingResult("Ошибка при добавлении производственной линии. ",
                     DatabaseQueryError);
    return;
  }

  processingResult("Новая линия производства успешно создана. ",
                   CompletedSuccessfully);
}

void AdministrationSystem::deleteLastProductionLines() {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Удаление последней линии производства. ");
  if (!removeLastProductionLine()) {
    processingResult(
        "Получена ошибка при удалении последней линии производства. ",
        DatabaseQueryError);
    return;
  }

  processingResult("Последняя линия производства успешно удалена. ",
                   CompletedSuccessfully);
}

void AdministrationSystem::loadSettings() {
}

bool AdministrationSystem::addOrder(
    const QMap<QString, QString>* orderParameters) const {
  QMap<QString, QString> issuerRecord;
  QMap<QString, QString> orderRecord;
  int32_t lastId = 0;

  issuerRecord.insert("id", "");
  issuerRecord.insert("name", orderParameters->value("IssuerName"));
  if (!Database->getRecordByPart("issuers", issuerRecord)) {
    emit logging(QString("Не найден идентифкатор эмитента \"%1\".")
                     .arg(orderParameters->value("IssuerName")));
    return false;
  }

  orderRecord.insert("id", "");
  if (!Database->getLastRecord("orders", orderRecord)) {
    emit logging("Ошибка при поиске последнего заказа. ");
    return false;
  }
  lastId = orderRecord.value("id").toInt();

  // Формируем новую запись
  orderRecord.insert("id", QString::number(lastId + 1));
  orderRecord.insert("issuer_id", issuerRecord.value("id"));
  orderRecord.insert("capacity", "0");
  orderRecord.insert("full_personalization",
                     orderParameters->value("FullPersonalization"));
  if (!Database->addRecord("orders", orderRecord)) {
    emit logging("Ошибка при добавлении заказа. ");
    return false;
  }

  return true;
}

bool AdministrationSystem::addPallets(
    const QMap<QString, QString>* orderParameters) const {
  uint32_t transponderCount =
      orderParameters->value("TransponderQuantity").toInt();
  uint32_t palletCapacity = orderParameters->value("PalletCapacity").toInt();
  uint32_t boxCapacity = orderParameters->value("BoxCapacity").toInt();
  uint32_t orderCapacity = transponderCount / (palletCapacity * boxCapacity);
  QMap<QString, QString> orderRecord;
  QMap<QString, QString> palletRecord;
  int32_t lastId = 0;

  // Поиск идентификатора незаполненного заказа
  orderRecord.insert("id", "");
  orderRecord.insert("capacity", "0");
  if (!Database->getRecordByPart("orders", orderRecord)) {
    emit logging("Ошибка при поиске идентификатора незаполненного заказа. ");
    return false;
  }

  // Заполняем заказ
  for (uint32_t i = 0; i < orderCapacity; i++) {
    // Получаем идентификатор последней палеты
    palletRecord.insert("id", "");
    if (!Database->getLastRecord("pallets", palletRecord)) {
      emit logging("Ошибка при поиске последней палеты. ");
      return false;
    }
    lastId = palletRecord.value("id").toInt();

    // Формируем новую запись
    palletRecord.insert("id", QString::number(lastId + 1));
    palletRecord.insert("capacity", "0");
    palletRecord.insert("order_id", orderRecord.value("id"));
    // Добавляем новую запись
    if (!Database->addRecord("pallets", palletRecord)) {
      emit logging("Ошибка при добавлении палеты. ");
      return false;
    }
  }

  // Заполнение заказа
  orderRecord.insert("capacity", QString::number(orderCapacity));
  if (!Database->updateRecord("orders", orderRecord)) {
    emit logging(QString("Ошибка при заполнении заказа %1. ")
                     .arg(orderRecord.value("id")));
    return false;
  }

  return true;
}

bool AdministrationSystem::addBoxes(
    const QMap<QString, QString>* orderParameters) const {
  uint32_t transponderCount =
      orderParameters->value("TransponderQuantity").toInt();
  uint32_t palletCapacity = orderParameters->value("PalletCapacity").toInt();
  uint32_t boxCapacity = orderParameters->value("BoxCapacity").toInt();
  uint32_t palletCount = transponderCount / (palletCapacity * boxCapacity);
  QMap<QString, QString> palletRecord;
  QMap<QString, QString> boxRecord;
  int32_t lastId = 0;

  for (uint32_t i = 0; i < palletCount; i++) {
    // Получаем идентификатор незаполненной палеты
    palletRecord.insert("id", "");
    palletRecord.insert("capacity", "0");
    if (!Database->getRecordByPart("pallets", palletRecord)) {
      emit logging("Ошибка при поиске идентификатора незаполненной палеты. ");
      return false;
    }

    // Создаем боксы
    for (uint32_t i = 0; i < palletCapacity; i++) {
      // Получаем идентификатор последнего добавленного бокса
      boxRecord.insert("id", "");
      if (!Database->getLastRecord("boxes", boxRecord)) {
        emit logging("Ошибка при поиске последнего бокса. ");
        return false;
      }
      lastId = boxRecord.value("id").toInt();

      // Формируем новую запись
      boxRecord.insert("id", QString::number(lastId + 1));
      boxRecord.insert("capacity", "0");
      boxRecord.insert("pallet_id", palletRecord.value("id"));
      // Добавляем новую запись
      if (!Database->addRecord("boxes", boxRecord)) {
        emit logging("Ошибка при добавлении бокса. ");
        return false;
      }
    }

    // Заполнение палеты
    palletRecord.insert("capacity", QString::number(palletCapacity));
    if (!Database->updateRecord("pallets", palletRecord)) {
      emit logging(QString("Ошибка при заполнении палеты %1. ")
                       .arg(palletRecord.value("id")));
      return false;
    }
  }

  return true;
}

bool AdministrationSystem::addTransponders(
    const QMap<QString, QString>* orderParameters) const {
  uint32_t transponderCount =
      orderParameters->value("TransponderQuantity").toInt();
  uint32_t boxCapacity = orderParameters->value("BoxCapacity").toInt();
  uint32_t boxCount = transponderCount / boxCapacity;
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> transponderRecord;
  int32_t lastId = 0;

  for (uint32_t i = 0; i < boxCount; i++) {
    // Получаем идентификатор незаполненного бокса
    boxRecord.insert("id", "");
    boxRecord.insert("capacity", "0");
    if (!Database->getRecordByPart("boxes", boxRecord)) {
      emit logging("Ошибка при поиске идентификатора незаполненного бокса. ");
      return false;
    }

    // Создаем транспондеры
    for (uint32_t i = 0; i < boxCapacity; i++) {
      // Получаем идентификатор последнего добавленного транспондера
      transponderRecord.insert("id", "");
      if (!Database->getLastRecord("transponders", transponderRecord)) {
        emit logging("Ошибка при поиске последнего бокса. ");
        return false;
      }
      lastId = transponderRecord.value("id").toInt();

      // Формируем новую запись
      transponderRecord.insert("id", QString::number(lastId + 1));
      transponderRecord.insert("model", "TC1001");
      transponderRecord.insert("payment_means", "0000000000000000000");
      transponderRecord.insert("release_counter", "0");
      transponderRecord.insert("box_id", boxRecord.value("id"));

      // Добавляем новую запись
      if (!Database->addRecord("transponders", transponderRecord)) {
        emit logging("Ошибка при добавлении транспондера. ");
        return false;
      }
    }

    // Заполнение бокса
    boxRecord.insert("capacity", QString::number(boxCapacity));
    if (!Database->updateRecord("boxes", boxRecord)) {
      emit logging(QString("Ошибка при заполнении бокса %1. ")
                       .arg(boxRecord.value("id")));
      return false;
    }
  }

  return true;
}

bool AdministrationSystem::addProductionLine(
    const QMap<QString, QString>* productionLineParameters) const {
  int32_t lastId = 0;
  QMap<QString, QString> productionLineRecord;
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  // Ищем первый транспондер в свободной коробке
  tables.append("transponders");
  tables.append("boxes");
  foreignKeys.append("box_id");
  mergedRecord.insert("transponders.id", "");
  mergedRecord.insert("transponders.box_id", "");
  mergedRecord.insert("boxes.in_process", "false");
  mergedRecord.insert("boxes.ready_indicator", "false");
  if (!Database->getMergedRecordByPart(tables, foreignKeys, mergedRecord)) {
    emit logging(
        "Получена ошибка при поиске первого транспондера свободного бокса. ");
    return false;
  }

  // Получаем идентификатор последней линии производства
  productionLineRecord.insert("id", "");
  if (!Database->getLastRecord("production_lines", productionLineRecord)) {
    emit logging("Ошибка при поиске последнего линии производства. ");
    return false;
  }
  lastId = productionLineRecord.value("id").toInt();

  // Формируем новую запись
  productionLineRecord = *productionLineParameters;
  productionLineRecord.insert("id", QString::number(lastId + 1));
  productionLineRecord.insert("transponder_id", mergedRecord.value("id"));
  if (!Database->addRecord("production_lines", productionLineRecord)) {
    emit logging("Получена ошибка при добавлении линии производства. ");
    return false;
  }

  // Запускаем сборку бокса
  if (!startBoxAssembling(mergedRecord.value("box_id"),
                          productionLineRecord.value("id"))) {
    return false;
  }

  return true;
}

bool AdministrationSystem::startBoxAssembling(
    const QString& id,
    const QString& productionLineId) const {
  QMap<QString, QString> boxRecord;

  boxRecord.insert("id", id);
  boxRecord.insert("pallet_id", "");
  boxRecord.insert("in_process", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging("Получена ошибка при поиске данных о боксе. ");
    return false;
  }

  if (boxRecord.value("in_process") != "true") {
    boxRecord.insert("in_process", "true");
    boxRecord.insert("production_line_id", productionLineId);
    if (!Database->updateRecord("boxes", boxRecord)) {
      emit logging("Получена ошибка при запуске сборки бокса. ");
      return false;
    }

    // Запуск сборки палеты
    if (!startPalletAssembling(boxRecord.value("pallet_id"))) {
      emit logging("Не удалось запустить сборку палеты. ");
      return false;
    }
  } else {
    emit logging(
        QString("Бокс %1 уже в процессе сборки. ").arg(boxRecord.value("id")));
  }
  return true;
}

bool AdministrationSystem::startPalletAssembling(const QString& id) const {
  QMap<QString, QString> palletRecord;

  palletRecord.insert("id", id);
  palletRecord.insert("order_id", "");
  palletRecord.insert("in_process", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    emit logging("Получена ошибка при поиске данных о паллете. ");
    return false;
  }

  if (palletRecord.value("in_process") != "true") {
    palletRecord.insert("in_process", "true");
    if (!Database->updateRecord("pallets", palletRecord)) {
      emit logging("Получена ошибка при запуске сборки палеты. ");
      return false;
    }

    // Запуск сборки заказа
    if (!startOrderAssembling(palletRecord.value("order_id"))) {
      emit logging("Не удалось запустить сборку заказа. ");
      return false;
    }
  } else {
    emit logging(QString("Палета %1 уже в процессе сборки. ")
                     .arg(palletRecord.value("id")));
  }

  return true;
}

bool AdministrationSystem::startOrderAssembling(const QString& id) const {
  QMap<QString, QString> orderRecord;

  orderRecord.insert("id", id);
  if (!Database->getRecordById("orders", orderRecord)) {
    emit logging("Получена ошибка при поиске данных о заказе. ");
    return false;
  }

  if (orderRecord.value("in_process") != "true") {
    orderRecord.insert("in_process", "true");
    if (!Database->updateRecord("orders", orderRecord)) {
      emit logging("Получена ошибка при запуске сборки заказа. ");
      return false;
    }
  } else {
    emit logging(QString("Заказ %1 уже в процессе сборки. ")
                     .arg(orderRecord.value("id")));
  }

  return true;
}

bool AdministrationSystem::removeLastProductionLine() const {
  QMap<QString, QString> productionLineRecord;
  QMap<QString, QString> boxRecord;

  // Получение последней добавленной линии производства
  productionLineRecord.insert("id", "");
  if (!Database->getLastRecord("production_lines", productionLineRecord)) {
    emit logging(
        "Ошибка при поиске последней добавленной производственной линии. ");
    return false;
  }

  // Получение бокса, соответствующий последней добавленной линии производства
  boxRecord.insert("id", "");
  boxRecord.insert("production_line_id", productionLineRecord.value("id"));
  if (!Database->getRecordByPart("boxes", boxRecord)) {
    emit logging(
        "Ошибка при поиске идентификатора бокса, соответствующего последней "
        "добавленной линии производства. ");
    return false;
  }

  // Удаляем запись из таблицы линий производства
  if (!Database->removeLastRecord("production_lines")) {
    emit logging("Получена ошибка при удалении линии производства. ");
    return false;
  }

  // Останавливаем сборку бокса
  emit logging(
      QString("Остановка сборки бокса %1. ").arg(boxRecord.value("id")));
  if (!stopBoxAssembling(boxRecord.value("id"))) {
    emit logging("Получена ошибка при остановке процесса сборки бокса. ");
    return false;
  }

  return true;
}

bool AdministrationSystem::stopBoxAssembling(const QString& id) const {
  QMap<QString, QString> boxRecord;
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  boxRecord.insert("id", id);
  boxRecord.insert("pallet_id", "");
  boxRecord.insert("in_process", "");
  if (!Database->getRecordById("boxes", boxRecord)) {
    emit logging("Получена ошибка при поиске данных о боксе. ");
    return false;
  }

  if (boxRecord.value("in_process") != "false") {
    boxRecord.insert("in_process", "false");
    if (!Database->updateRecord("boxes", boxRecord)) {
      emit logging("Получена ошибка при остановке сборки бокса. ");
      return false;
    }

    // Проверка наличия в палете других боксов , находящихся в процессе сборки
    tables.append("boxes");
    tables.append("pallets");
    mergedRecord.insert("boxes.id", "");
    mergedRecord.insert("boxes.in_process", "true");
    mergedRecord.insert("boxes.pallet_id", boxRecord.value("pallet_id"));
    foreignKeys.append("pallet_id");
    if (Database->getMergedRecordByPart(tables, foreignKeys, mergedRecord)) {
      // Проверка наличия записей
      if (mergedRecord.isEmpty()) {
        // Остановка сборки паллеты
        emit logging(QString("Остановка сборки паллеты %1. ")
                         .arg(boxRecord.value("pallet_id")));
        if (!stopPalletAssembling(boxRecord.value("pallet_id"))) {
          emit logging("Не удалось остановить сборку палеты. ");
          return false;
        }
      } else {
        emit logging(
            "В паллете еще есть боксы, находящиеся в процессе сборки. ");
      }
    }
  } else {
    emit logging(QString("Бокс %1 не находится в процессе сборки. ")
                     .arg(boxRecord.value("id")));
  }
  return true;
}

bool AdministrationSystem::stopPalletAssembling(const QString& id) const {
  QMap<QString, QString> palletRecord;
  QMap<QString, QString> mergedRecord;
  QStringList tables;
  QStringList foreignKeys;

  palletRecord.insert("id", id);
  palletRecord.insert("order_id", "");
  palletRecord.insert("in_process", "");
  if (!Database->getRecordById("pallets", palletRecord)) {
    emit logging("Получена ошибка при поиске данных о паллете. ");
    return false;
  }

  if (palletRecord.value("in_process") != "false") {
    palletRecord.insert("in_process", "false");
    if (!Database->updateRecord("pallets", palletRecord)) {
      emit logging("Получена ошибка при остановке сборки палеты. ");
      return false;
    }

    // Проверка наличия в заказе других паллет, находящихся в процессе сборки
    tables.append("pallets");
    tables.append("orders");
    mergedRecord.insert("pallets.id", "");
    mergedRecord.insert("pallets.in_process", "true");
    mergedRecord.insert("pallets.order_id", palletRecord.value("order_id"));
    foreignKeys.append("order_id");
    if (Database->getMergedRecordByPart(tables, foreignKeys, mergedRecord)) {
      // Проверка наличия записей
      if (mergedRecord.isEmpty()) {
        // Остановка сборки заказа
        emit logging(QString("Остановка сборки заказа %1. ")
                         .arg(palletRecord.value("order_id")));
        if (!stopOrderAssembling(palletRecord.value("order_id"))) {
          emit logging("Не удалось остановить сборку заказа. ");
          return false;
        }
      } else {
        emit logging(
            "В заказе еще есть паллеты, находящиеся в процессе сборки. ");
      }
    }
  } else {
    emit logging(QString("Палета %1 не находится в процессе сборки. ")
                     .arg(palletRecord.value("id")));
  }

  return true;
}

bool AdministrationSystem::stopOrderAssembling(const QString& id) const {
  QMap<QString, QString> orderRecord;

  orderRecord.insert("id", id);
  if (!Database->getRecordById("orders", orderRecord)) {
    emit logging("Получена ошибка при поиске данных о заказе. ");
    return false;
  }

  if (orderRecord.value("in_process") != "false") {
    orderRecord.insert("in_process", "false");
    if (!Database->updateRecord("orders", orderRecord)) {
      emit logging("Получена ошибка при остановке сборки заказа. ");
      return false;
    }
  } else {
    emit logging(QString("Заказ %1 не находится в процессе сборки. ")
                     .arg(orderRecord.value("id")));
  }

  return true;
}

void AdministrationSystem::processingResult(const QString& log,
                                            const ExecutionStatus status) {
  emit logging(log);
  emit logging("Отключение от базы данных. ");

  if (status == CompletedSuccessfully) {
    Database->disconnect(IDatabaseController::Complete);
  } else {
    Database->disconnect(IDatabaseController::Abort);
  }
  emit operationFinished(status);
}
