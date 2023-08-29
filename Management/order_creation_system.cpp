#include "order_creation_system.h"

OrderSystem::OrderSystem(QObject* parent) : QObject(parent) {
  setObjectName("OrderSystem");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");

  loadSettings();

  createDatabaseController();
}

void OrderSystem::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController") {
    if (DatabaseLogOption) {
      emit logging("Postgres controller - " + log);
    }
  } else {
    emit logging("Unknown - " + log);
  }
}

void OrderSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  loadSettings();
  Database->applySettings();
}

void OrderSystem::createDatabaseController() {
  // Создаем контроллер базы данных
  Database = new PostgresController(this, "OrderCreatorConnection");
  connect(Database, &IDatabaseController::logging, this,
          &OrderSystem::proxyLogging);
}

void OrderSystem::clearDatabaseTable(const QString& tableName) {
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

void OrderSystem::getDatabaseTable(const QString& tableName,
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

void OrderSystem::getCustomResponse(const QString& req,
                                    DatabaseTableModel* buffer) {
  Database->execCustomRequest(req, buffer);
}

void OrderSystem::createNewOrder(IssuerOrder* order) {
  emit logging("Подключение к базе данных. ");
  if (!Database->connect()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Добавление заказа. ");
  if (!addOrder(order)) {
    processingResult("Получена ошибка при добавлении заказа. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление палет. ");
  if (!addPallets(order)) {
    processingResult("Получена ошибка при добавлении палет. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление боксов. ");
  if (!addBoxes(order)) {
    processingResult("Получена ошибка при добавлении боксов. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление транспондеров. ");
  if (!addTransponders(order)) {
    processingResult("Получена ошибка при добавлении транспондеров. ",
                     DatabaseQueryError);
    return;
  }

  processingResult("Новый заказ успешно создан. ", CompletedSuccessfully);
}

void OrderSystem::deleteLastOrder() {
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

void OrderSystem::initIssuerTable() {
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

void OrderSystem::loadSettings() {
  QSettings settings;

  DatabaseLogOption = settings.value("Database/Log/Active").toBool();
}

bool OrderSystem::addOrder(IssuerOrder* order) {
  QMap<QString, QString> record;
  QPair<QString, QString> attribute;
  int32_t lastId = 0;

  // Получение идентификатора эмитента по его имени
  attribute.first = "Name";
  attribute.second = order->issuerName();
  int32_t issuerId = Database->getIdByAttribute("issuers", attribute);
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
                order->fullPersonalization() ? "true" : "false");
  record.insert("ProductionStartDate",
                order->productionStartDate().toString("dd.MM.yyyy"));
  emit logging("Добавление нового заказа. ");
  if (!Database->addRecord("orders", record)) {
    return false;
  }

  return true;
}

bool OrderSystem::addPallets(IssuerOrder* order) {
  QMap<QString, QString> record;
  uint32_t transponderCount = order->transponderQuantity();
  uint32_t palletCount =
      transponderCount / (BOX_TRANSPONDER_QUANTITY * PALLET_BOX_QUANTITY);
  int32_t orderId = 0;
  int32_t lastId = 0;

  // Получаем идентификатор незаполненного заказа
  orderId =
      Database->getIdByCondition("orders", "\"TotalPalletQuantity\" = 0", true);
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

bool OrderSystem::addBoxes(IssuerOrder* order) {
  QMap<QString, QString> record;
  uint32_t transponderCount = order->transponderQuantity();
  uint32_t palletCount =
      transponderCount / (BOX_TRANSPONDER_QUANTITY * PALLET_BOX_QUANTITY);
  int32_t palletId = 0;
  int32_t lastId = 0;

  for (uint32_t i = 0; i < palletCount; i++) {
    // Получаем идентификатор незаполненной палеты
    palletId =
        Database->getIdByCondition("pallets", "\"TotalBoxQuantity\" = 0", true);
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

bool OrderSystem::addTransponders(IssuerOrder* order) {
  QMap<QString, QString> record;
  QPair<QString, QString> attribute;
  uint32_t transponderCount = order->transponderQuantity();
  uint32_t boxCount = transponderCount / BOX_TRANSPONDER_QUANTITY;
  int32_t boxId = 0;
  int32_t lastId = 0;

  for (uint32_t i = 0; i < boxCount; i++) {
    // Получаем идентификатор незаполненного бокса
    boxId = Database->getIdByCondition(
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

void OrderSystem::processingResult(const QString& log,
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

void OrderSystem::init() {}

OCSBuilder::OCSBuilder() : QObject(nullptr) {
  // Пока никакие объекты не созданы
  BuildedObject = nullptr;
}

OrderSystem* OCSBuilder::buildedObject() const {
  return BuildedObject;
}

void OCSBuilder::build() {
  BuildedObject = new OrderSystem(nullptr);

  emit completed();
}
