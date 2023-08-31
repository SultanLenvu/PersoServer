#include "administration_system.h"

AdministrationSystem::AdministrationSystem(QObject* parent) : QObject(parent) {
  setObjectName("AdministrationSystem");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");

  loadSettings();

  createDatabaseController();
}

void AdministrationSystem::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController") {
    if (DatabaseLogOption) {
      emit logging("Postgres controller - " + log);
    }
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
  Database->execCustomRequest(req, buffer);
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
      "orders", "(\"InProcess\" = 'false' AND \"ReadyIndicator\" = 'false')");

  processingResult("Последний добавленный заказ успешно удален. ",
                   CompletedSuccessfully);
}

void AdministrationSystem::createNewProductionLine(
    const QMap<QString, QString>* productionLineParameters) {
  QMap<QString, QString> transponderRecord;

  emit logging("Создание новой линии производства. ");

  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  // Запуск сборки бокса
  if (!startBoxAssembling(transponderRecord)) {
    processingResult("Не удалось запустить сборку бокса. ", DatabaseQueryError);
    return;
  }

  // Добавляем линию производства
  if (!addProductionLine(productionLineParameters,
                         transponderRecord.value("Id"))) {
    processingResult("Не удалось запустить сборку палеты. ",
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
  if (!Database->removeLastRecord("production_lines")) {
    processingResult("Получена ошибка при удалении линии производства. ",
                     DatabaseQueryError);
    return;
  }

  processingResult("Последний добавленный заказ успешно удален. ",
                   CompletedSuccessfully);
}

void AdministrationSystem::initIssuerTable() {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  QMap<QString, QString> record;

  emit logging("Инициализация таблицы эмитентов. ");
  record.insert("Name", "Пауэр Синтез");
  record.insert("EfcContextMark", "000000000001");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Автодор");
  record.insert("EfcContextMark", "570002FF0070");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Новое качество дорог");
  record.insert("EfcContextMark", "000000000001");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Западный скоростной диаметр");
  record.insert("EfcContextMark", "570001FF0070");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Объединенные системы сбора платы");
  record.insert("EfcContextMark", "000000000001");
  if (!Database->addRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  processingResult("Инициализация успешно завершена. ", CompletedSuccessfully);
}

void AdministrationSystem::loadSettings() {
  QSettings settings;

  DatabaseLogOption = settings.value("Database/Log/Active").toBool();
}

bool AdministrationSystem::addOrder(
    const QMap<QString, QString>* orderParameters) {
  QMap<QString, QString> record;
  QPair<QString, QString> attribute;
  int32_t lastId = 0;

  // Получение идентификатора эмитента по его имени
  attribute.first = "Name";
  attribute.second = orderParameters->value("IssuerName");
  int32_t issuerId = Database->getFirstIdByAttribute("issuers", attribute);
  if (issuerId == -1) {
    return false;
  }

  // Получаем идентифкатор последнего добавленного заказа
  lastId = Database->getLastId("orders");
  if (lastId == -1) {
    return false;
  }

  // Формируем новую запись
  record.insert("Id", QString::number(lastId + 1));
  record.insert("IssuerId", QString::number(issuerId));
  record.insert("TotalPalletQuantity", "0");
  record.insert("FullPersonalization",
                orderParameters->value("FullPersonalization"));
  record.insert("ProductionStartDate",
                QDate::currentDate().toString("dd.MM.yyyy"));
  record.insert("ProductionEndDate",
                QDate::currentDate().toString("dd.MM.yyyy"));
  emit logging("Добавление нового заказа. ");
  if (!Database->addRecord("orders", record)) {
    return false;
  }

  return true;
}

bool AdministrationSystem::addPallets(
    const QMap<QString, QString>* orderParameters) {
  QMap<QString, QString> record;
  uint32_t transponderCount =
      orderParameters->value("TransponderQuantity").toInt();
  uint32_t palletCount =
      transponderCount / (BOX_TRANSPONDER_QUANTITY * PALLET_BOX_QUANTITY);
  int32_t orderId = 0;
  int32_t lastId = 0;

  // Получаем идентификатор незаполненного заказа
  orderId = Database->getFirstIdWithCondition(
      "orders", "\"TotalPalletQuantity\" = 0", true);
  if (orderId == -1) {
    return false;
  }

  // Заполняем заказ
  for (uint32_t i = 0; i < palletCount; i++) {
    // Получаем идентификатор последней добавленной палеты
    lastId = Database->getLastId("pallets");
    if (lastId == -1) {
      return false;
    }

    // Формируем новую запись
    record.insert("Id", QString::number(lastId + 1));
    record.insert("TotalBoxQuantity", "0");
    record.insert("OrderId", QString::number(orderId));

    // Добавляем новую запись
    if (!Database->addRecord("pallets", record)) {
      return false;
    }

    if (!Database->increaseAttributeValue("orders", "TotalPalletQuantity",
                                          QString::number(orderId), 1)) {
      return false;
    }
  }

  return true;
}

bool AdministrationSystem::addBoxes(
    const QMap<QString, QString>* orderParameters) {
  QMap<QString, QString> record;
  uint32_t transponderCount =
      orderParameters->value("TransponderQuantity").toInt();
  uint32_t palletCount =
      transponderCount / (BOX_TRANSPONDER_QUANTITY * PALLET_BOX_QUANTITY);
  int32_t palletId = 0;
  int32_t lastId = 0;

  for (uint32_t i = 0; i < palletCount; i++) {
    // Получаем идентификатор незаполненной палеты
    palletId = Database->getFirstIdWithCondition(
        "pallets", "\"TotalBoxQuantity\" = 0", true);
    if (palletId == -1) {
      return false;
    }

    // Заполняем палету
    for (uint32_t i = 0; i < PALLET_BOX_QUANTITY; i++) {
      // Получаем идентификатор последнего добавленного бокса
      lastId = Database->getLastId("boxes");
      if (lastId == -1) {
        return false;
      }

      // Формируем новую запись
      record.insert("Id", QString::number(lastId + 1));
      record.insert("TotalTransponderQuantity", "0");
      record.insert("PalletId", QString::number(palletId));

      // Добавляем новую запись
      if (!Database->addRecord("boxes", record)) {
        return false;
      }
      if (!Database->increaseAttributeValue("pallets", "TotalBoxQuantity",
                                            QString::number(palletId), 1)) {
        return false;
      }
    }
  }

  return true;
}

bool AdministrationSystem::addTransponders(
    const QMap<QString, QString>* orderParameters) {
  QMap<QString, QString> record;
  QPair<QString, QString> attribute;
  uint32_t transponderCount =
      orderParameters->value("TransponderQuantity").toInt();
  uint32_t boxCount = transponderCount / BOX_TRANSPONDER_QUANTITY;
  int32_t boxId = 0;
  int32_t lastId = 0;

  for (uint32_t i = 0; i < boxCount; i++) {
    // Получаем идентификатор незаполненного бокса
    boxId = Database->getFirstIdWithCondition(
        "boxes", "\"TotalTransponderQuantity\" = 0", true);
    if (boxId == -1) {
      return false;
    }

    // Заполняем бокс
    for (uint32_t i = 0; i < BOX_TRANSPONDER_QUANTITY; i++) {
      // Получаем идентификатор последнего добавленного транспондера
      lastId = Database->getLastId("transponders");
      if (lastId == -1) {
        return false;
      }

      // Формируем новую запись
      record.insert("Id", QString::number(lastId + 1));
      record.insert("Model", "TC1001");
      record.insert("PaymentMeans", "0000000000000000000");
      record.insert("EmissionCounter", "0");
      record.insert("BoxId", QString::number(boxId));

      // Добавляем новую запись
      if (!Database->addRecord("transponders", record)) {
        return false;
      }
      if (!Database->increaseAttributeValue("boxes", "TotalTransponderQuantity",
                                            QString::number(boxId), 1)) {
        return false;
      }
    }
  }

  return true;
}

bool AdministrationSystem::addProductionLine(
    const QMap<QString, QString>* productionLineParameters,
    const QString& transponderId) {
  int32_t lastId = 0;
  QMap<QString, QString> productionLineRecord;

  // Получаем идентификатор последней линии производства
  lastId = Database->getLastId("production_lines");
  if (lastId == -1) {
    processingResult(
        "Получена ошибка при поиске последней линии производства. ",
        DatabaseQueryError);
    return false;
  }

  // Формируем новую запись
  productionLineRecord = *productionLineParameters;
  productionLineRecord.insert("Id", QString::number(lastId + 1));
  productionLineRecord.insert("TransponderId", transponderId);
  emit logging("Добавление линии производства. ");
  if (!Database->addRecord("production_lines", productionLineRecord)) {
    processingResult("Получена ошибка при добавлении линии производства. ",
                     DatabaseQueryError);
    return false;
  }

  return true;
}

bool AdministrationSystem::startBoxAssembling(
    QMap<QString, QString>& transponderRecord) {
  QMap<QString, QString> boxRecord;

  emit logging("Поиск свободного бокса. ");
  boxRecord.insert("InProcess", "false");
  boxRecord.insert("ReadyIndicator", "false");
  if (!Database->getRecordByPart("boxes", boxRecord)) {
    emit logging("Свободных боксов нет. ");
    return false;
  }

  emit logging(
      QString("Запуск бокса %1 в процесс сборки. ").arg(boxRecord.value("Id")));
  boxRecord.insert("InProcess", "true");
  if (!Database->updateRecord("boxes", boxRecord)) {
    emit logging("Получена ошибка при запуске бокса в процесс сборки. ");
    return false;
  }

  emit logging(QString("Поиск первого транспондера в боксе %1. ")
                   .arg(boxRecord.value("Id")));
  transponderRecord.insert("BoxId", boxRecord.value("Id"));
  if (!Database->getRecordByPart("transponders", transponderRecord)) {
    emit logging(QString("В боксе %1 отсутствуют транспондеры. ")
                     .arg(boxRecord.value("Id")));
    return false;
  }

  // Запуск сборки палеты
  if (!startPalletAssembling(boxRecord)) {
    emit logging("Не удалось запустить сборку палеты. ");
    return false;
  }

  return true;
}

bool AdministrationSystem::startPalletAssembling(
    const QMap<QString, QString>& boxRecord) {
  QMap<QString, QString> palletRecord;

  emit logging(
      QString("Поиск палеты, содержащей бокс %1. ").arg(boxRecord.value("Id")));
  if (!Database->getRecordById("pallets", boxRecord.value("PalletId").toInt(),
                               palletRecord)) {
    return false;
  }

  // Если бокс палеты в процессе сборки, то сама палета также должна быть в
  // процесс сборки
  if (palletRecord.value("InProcess") == "false") {
    emit logging(
        QString("Запуск сборки паллеты %1. ").arg(palletRecord.value("Id")));
    palletRecord.insert("InProcess", "true");
    if (!Database->updateRecord("pallets", palletRecord)) {
      processingResult("Получена ошибка при запуске бокса в процесс сборки. ",
                       DatabaseQueryError);
      return false;
    }

    // Запуск сборки заказа
    if (!startOrderAssembling(palletRecord)) {
      emit logging("Не удалось запустить сборку заказа. ");
      return false;
    }
  } else {
    emit logging(QString("Паллета %1 уже в процессе сборки. ")
                     .arg(palletRecord.value("Id")));
  }

  return true;
}

bool AdministrationSystem::startOrderAssembling(
    const QMap<QString, QString>& palletRecord) {
  QMap<QString, QString> orderRecord;

  emit logging(QString("Поиск заказа, содержащего палету %1. ")
                   .arg(palletRecord.value("Id")));
  if (!Database->getRecordById("orders", palletRecord.value("OrderId").toInt(),
                               orderRecord)) {
    return false;
  }

  // Если палета заказа в процессе сборки, то сам заказ также должен быть в
  // процесс сборки
  if (orderRecord.value("InProcess") == "false") {
    emit logging(QString("Запуск заказа %1 в процесс сборки. ")
                     .arg(palletRecord.value("Id")));
    orderRecord.insert("InProcess", "true");
    if (!Database->updateRecord("orders", orderRecord)) {
      return false;
    }
  }

  return true;
}

void AdministrationSystem::processingResult(const QString& log,
                                            const ExecutionStatus status) {
  emit logging(log);
  emit logging("Отключение от базы данных. ");

  if (status == CompletedSuccessfully) {
    Database->disconnect(true);
  } else {
    Database->disconnect(false);
  }
  emit operationFinished(status);
}
