#include "order_creation_system.h"

OrderSystem::OrderSystem(QObject* parent) : QObject(parent) {
  setObjectName("OrderSystem");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");

  createDatabaseController();
}

void OrderSystem::proxyLogging(const QString& log) {
  //  if (sender()->objectName() == "PostgresController")
  //    emit logging("Postgres controller - " + log);
  //  else
  //    emit logging("Unknown - " + log);
}

void OrderSystem::applySettings() {
  emit logging("Применение новых настроек. ");
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
  Database->connect();

  if (!Database->isConnected()) {
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

  emit logging("Очистка данных таблицы базы данных. ");
  Database->clearTable(tableName);

  emit logging("Отключение от базы данных. ");
  Database->disconnect();
  emit operationFinished(CompletedSuccessfully);
}

void OrderSystem::getDatabaseTable(const QString& tableName,
                                   DatabaseTableModel* buffer) {
  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

  emit logging("Получение таблицы базы данных. ");
  Database->getTable(tableName, 10, buffer);

  emit logging("Отключение от базы данных. ");
  Database->disconnect();

  if (buffer->isEmpty()) {
    emit logging("Ошибка при выполнении запроса к базе данных. ");
    emit operationFinished(DatabaseQueryError);
  } else {
    emit logging("Операция успешно выполнена. ");
    emit operationFinished(CompletedSuccessfully);
  }
}

void OrderSystem::getCustomResponse(const QString& req,
                                    DatabaseTableModel* buffer) {
  Database->execCustomRequest(req, buffer);
}

void OrderSystem::createNewOrder(IssuerOrder* order) {
  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Добавление заказа. ");
  if (!addOrder(order)) {
    return;
  }

  emit logging("Добавление палет. ");
  if (!addPallets(order)) {
    return;
  }

  emit logging("Добавление боксов. ");
  if (!addBoxes(order)) {
    return;
  }

  emit logging("Добавление транспондеров. ");
  if (!addTransponders(order)) {
    return;
  }

  processingResult("Новый заказ успешно создан. ", CompletedSuccessfully);
}

void OrderSystem::deleteLastOrder() {
  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    processingResult("Не удалось установить соединение с базой данных. ",
                     DatabaseConnectionError);
    return;
  }

  emit logging("Удаление последнего добавленного заказа. ");
  Database->removeTableLastRecord("orders");

  processingResult("Новый заказ успешно создан. ", CompletedSuccessfully);
}

void OrderSystem::initIssuerTable() {
  QMap<QString, QString> record;
  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

  emit logging("Инициализация таблицы эмитентов. ");
  record.insert("Name", "Пауэр Синтез");
  record.insert("EfcContextMark", "000000000001");
  if (!Database->addTableRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Автодор");
  record.insert("EfcContextMark", "570002FF0070");
  if (!Database->addTableRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Новое качество дорог");
  record.insert("EfcContextMark", "000000000001");
  if (!Database->addTableRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Западный скоростной диаметр");
  record.insert("EfcContextMark", "570001FF0070");
  if (!Database->addTableRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  record.insert("Name", "Объединенные системы сбора платы");
  record.insert("EfcContextMark", "000000000001");
  if (!Database->addTableRecord("issuers", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.clear();

  processingResult("Инициализация успешно завершена. ", CompletedSuccessfully);
}

bool OrderSystem::addOrder(IssuerOrder* order) {
  QMap<QString, QString> record;
  QPair<QString, QString> attribute;

  attribute.first = "Name";
  attribute.second = order->issuerName();
  int32_t issuerId = Database->getIdByAttribute("issuers", attribute);
  if (issuerId == -1) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return false;
  }

  record.insert("IssuerId", QString::number(issuerId));
  record.insert("TotalPalletQuantity", "0");
  record.insert("FullPersonalization",
                order->fullPersonalization() ? "true" : "false");
  record.insert("ProductionStartDate",
                order->productionStartDate().toString("dd.MM.yyyy"));
  emit logging("Добавление нового заказа. ");
  if (!Database->addTableRecord("orders", record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
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

  // Получаем идентификатор заказа
  orderId =
      Database->getIdByCondition("orders", "\"TotalPalletQuantity\" = 0", true);
  if (orderId == -1) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return false;
  }

  record.insert("TotalBoxQuantity", "0");
  record.insert("OrderId", QString::number(orderId));
  for (uint32_t i = 0; i < palletCount; i++) {
    if (!Database->addTableRecord("pallets", record)) {
      processingResult("Ошибка при выполнении запроса в базу данных. ",
                       DatabaseQueryError);
      return false;
    }

    if (!Database->increaseAttributeValue("orders", "TotalPalletQuantity",
                                          QString::number(orderId), 1)) {
      processingResult("Ошибка при выполнении запроса в базу данных. ",
                       DatabaseQueryError);
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

  for (uint32_t i = 0; i < palletCount; i++) {
    palletId =
        Database->getIdByCondition("pallets", "\"TotalBoxQuantity\" = 0", true);
    if (palletId == -1) {
      processingResult("Ошибка при выполнении запроса в базу данных. ",
                       DatabaseQueryError);
      return false;
    }

    record.insert("TotalTransponderQuantity", "0");
    record.insert("PalletId", QString::number(palletId));
    for (uint32_t i = 0; i < PALLET_BOX_QUANTITY; i++) {
      if (!Database->addTableRecord("boxes", record)) {
        processingResult("Ошибка при выполнении запроса в базу данных. ",
                         DatabaseQueryError);
        return false;
      }
      if (!Database->increaseAttributeValue("pallets", "TotalBoxQuantity",
                                            QString::number(palletId), 1)) {
        processingResult("Ошибка при выполнении запроса в базу данных. ",
                         DatabaseQueryError);
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

  for (uint32_t i = 0; i < boxCount; i++) {
    // Получаем идентификатор бокса
    boxId = Database->getIdByCondition(
        "boxes", "\"TotalTransponderQuantity\" = 0", true);
    if (boxId == -1) {
      processingResult("Ошибка при выполнении запроса в базу данных. ",
                       DatabaseQueryError);
      return false;
    }

    record.insert("Model", "TC1001");
    record.insert("PaymentMeans", "0000000000000000000");
    record.insert("EmissionCounter", "0");
    record.insert("BoxId", QString::number(boxId));
    for (uint32_t i = 0; i < BOX_TRANSPONDER_QUANTITY; i++) {
      if (!Database->addTableRecord("transponders", record)) {
        processingResult("Ошибка при выполнении запроса в базу данных. ",
                         DatabaseQueryError);
        return false;
      }
      if (!Database->increaseAttributeValue("boxes", "TotalTransponderQuantity",
                                            QString::number(boxId), 1)) {
        processingResult("Ошибка при выполнении запроса в базу данных. ",
                         DatabaseQueryError);
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
  Database->disconnect();
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
