#include "order_creation_system.h"

OrderCreationSystem::OrderCreationSystem(QObject* parent) : QObject(parent) {
  setObjectName("OrderCreationSystem");

  qRegisterMetaType<ExecutionStatus>("ExecutionStatus");

  createDatabaseController();
}

void OrderCreationSystem::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres controller - " + log);
  else
    emit logging("Unknown - " + log);
}

void OrderCreationSystem::applySettings() {
  emit logging("Применение новых настроек. ");
  Database->applySettings();
}

void OrderCreationSystem::createDatabaseController() {
  // Создаем контроллер базы данных
  Database = new PostgresController(this, "OrderCreatorConnection");
  connect(Database, &IDatabaseController::logging, this,
          &OrderCreationSystem::proxyLogging);
}

void OrderCreationSystem::getDatabaseTable(const QString& tableName,
                                           DatabaseBuffer* buffer) {
  if (QApplication::instance()->thread() != thread()) {
    emit logging("Операция выполняется в отдельном потоке. ");
  } else {
    emit logging("Операция выполняется в главном потоке. ");
  }

  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

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

void OrderCreationSystem::getCustomResponse(const QString& req,
                                            DatabaseBuffer* buffer) {
  Database->execCustomRequest(req, buffer);
}

void OrderCreationSystem::createNewOrder(IssuerOrder* order) {
  emit logging("Подключение к базе данных. ");
  Database->connect();

  if (!Database->isConnected()) {
    emit logging("Соединение с базой данных не установлено. ");
    emit operationFinished(DatabaseConnectionError);
    return;
  }

  emit logging("Создание нового заказа. ");

  if (!Database->addOrderToIssuer(order->issuerName())) {
    emit logging("Отключение от базы данных. ");
    Database->disconnect();
    emit logging("Ошибка при выполнении запроса к базе данных. ");
    emit operationFinished(DatabaseQueryError);
    return;
  }

  emit logging("Операция успешно выполнена. ");
  emit operationFinished(CompletedSuccessfully);
}

void OrderCreationSystem::init() {}

OCSBuilder::OCSBuilder() : QObject(nullptr) {
  // Пока никакие объекты не созданы
  BuildedObject = nullptr;
}

OrderCreationSystem* OCSBuilder::buildedObject() const {
  return BuildedObject;
}

void OCSBuilder::build() {
  BuildedObject = new OrderCreationSystem(nullptr);

  emit completed();
}
