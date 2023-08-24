#include "order_creation_system.h"

OrderSystem::OrderSystem(QObject* parent) : QObject(parent) {
  setObjectName("OrderSystem");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");

  createDatabaseController();
}

void OrderSystem::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres controller - " + log);
  else
    emit logging("Unknown - " + log);
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
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

  emit logging("Создание нового заказа. ");
  OrderRecord record(nullptr);

  emit logging("Формирование записи в таблицу заказов. ");
  uint32_t issuerId = 0;
  if (!Database->getIssuerId("Новое качество дорог", issuerId)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }
  record.setIssuerId(issuerId);
  record.setTransponderQuantity(order->transponderQuantity());
  record.setFullPersonalization(order->fullPersonalization());
  record.setProductionStartDate(order->productionStartDate());

  emit logging("Добавление новой записи в таблицу заказов. ");
  if (!Database->addOrder(record)) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }

  emit logging("Добавление заказа эмитенту. ");
  if (!Database->addOrderToIssuer(order->issuerName())) {
    processingResult("Ошибка при выполнении запроса в базу данных. ",
                     DatabaseQueryError);
    return;
  }

  processingResult("Новый заказ успешно создан. ", CompletedSuccessfully);
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
