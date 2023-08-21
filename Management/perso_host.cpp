#include "perso_host.h"

PersoHost::PersoHost(QObject* parent, QSettings* settings)
    : QTcpServer(parent) {
  setObjectName("PersoHost");

  Settings = settings;

  PauseIndicator = false;

  // Интерфейс для централизованного доступа к базе данных
  Database = new PostgresController(this, QString("ServerConnection"));
  connect(Database, &DatabaseControllerInterface::logging, this,
          &PersoHost::proxyLogging);
  // Настраиваем контроллер базы данных
  Database->applySettings(Settings);
}

PersoHost::~PersoHost() {
  emit logging("Уничтожен. ");
}

void PersoHost::start() {
  // Поднимаем сервер
  if (!listen(QHostAddress::LocalHost, 6666)) {
    emit logging("Не удалось запуститься. ");
    return;
  }

  // Если сервер поднялся
  emit logging("Запущен. ");
  if (thread() == QCoreApplication::instance()->thread())
    emit logging("Сервер запущен в главном потоке. ");
  else
    emit logging("Сервер запущен в отдельном потоке. ");

  // Подключаемся к базе данных
  Database->connect();
}

void PersoHost::stop() {
  close();
  emit logging("Остановлен. ");
}

void PersoHost::getProductionLines(DatabaseBuffer* buffer) {
  Database->getTable("ProductionLines", 10, buffer);
}

void PersoHost::getTransponders(DatabaseBuffer* buffer) {
  Database->getTable("Transponders", 10, buffer);
}

void PersoHost::getOrders(DatabaseBuffer* buffer) {
  Database->getTable("Orders", 10, buffer);
}

void PersoHost::getIssuers(DatabaseBuffer* buffer) {
  Database->getTable("Issuers", 10, buffer);
}

void PersoHost::getBoxes(DatabaseBuffer* buffer) {
  Database->getTable("Boxes", 10, buffer);
}

void PersoHost::getPallets(DatabaseBuffer* buffer) {
  Database->getTable("Pallets", 10, buffer);
}

void PersoHost::getCustomResponse(const QString& req, DatabaseBuffer* buffer) {
  Database->execCustomRequest(req, buffer);
}

void PersoHost::incomingConnection(qintptr socketDescriptor) {
  emit logging("Получен запрос на новое подключение. ");

  // Создаем среду выполнения для клиента
  createClientInstance(socketDescriptor);

  // Проверяем созданную среду выполнения
  emit checkNewClientInstance();

  // Если достигнут лимит подключений
  if (Clients.size() == CLIENT_MAX_COUNT) {
    pauseAccepting();  // Блокируем прием новых подключений
    PauseIndicator = true;

    emit logging("Достигнут лимит подключений, прием новых приостановлен. ");
  }
}

void PersoHost::createClientInstance(qintptr socketDescriptor) {
  // Создаем новое клиент-подключение
  PersoClientConnection* newClient =
      new PersoClientConnection(Clients.size(), socketDescriptor, Settings);

  connect(newClient, &PersoClientConnection::logging, this,
          &PersoHost::proxyLogging);
  connect(newClient, &PersoClientConnection::disconnected, this,
          &PersoHost::on_ClientDisconnected_slot);
  connect(this, &PersoHost::checkNewClientInstance, newClient,
          &PersoClientConnection::instanceTesting);

  // Добавляем клиента в реестр
  Clients.append(newClient);
  emit logging(QString("Новый клиент создан и зарегистрирован в реестре с "
                       "идентификатором %1. ")
                   .arg(QString::number(Clients.last()->getId())));

  // Создаем отдельный поток для клиента
  QThread* newClientThread = new QThread(this);
  newClient->moveToThread(newClientThread);

  connect(newClient, &PersoClientConnection::disconnected, newClientThread,
          &QThread::quit);
  connect(newClientThread, &QThread::finished, newClientThread,
          &QThread::deleteLater);
  connect(newClientThread, &QThread::finished, newClient,
          &PersoClientConnection::deleteLater);
  connect(newClient, &PersoClientConnection::destroyed, this,
          &PersoHost::on_ClientConnectionDeleted_slot);
  connect(newClientThread, &QThread::destroyed, this,
          &PersoHost::on_ClientThreadDeleted_slot);

  // Добавляем поток в соответствующий реестр
  ClientThreads.append(newClientThread);

  // Запускаем поток
  newClientThread->start();
  emit logging("Клиентский поток запущен. ");
}

void PersoHost::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PersoClientConnection")
    emit logging(
        QString("Client %1 - ")
            .arg(QString::number(
                dynamic_cast<PersoClientConnection*>(sender())->getId())) +
        log);
  else if (sender()->objectName() == "PostgresController")
    emit logging("Postgres - " + log);
  else
    emit logging("Unknown - " + log);
}

void PersoHost::on_ClientDisconnected_slot() {
  uint32_t clientId = dynamic_cast<PersoClientConnection*>(sender())->getId();

  // Удаляем отключившегося клиента и его поток из соответствующих реестров
  for (int32_t i = 0; i < Clients.size(); i++) {
    if (clientId == Clients.at(i)->getId()) {
      Clients.removeAt(clientId);
      ClientThreads.removeAt(clientId);
      break;
    }
  }

  emit logging(
      QString("Клиент %1 удален из реестра. ").arg(QString::number(clientId)));

  // Если ранее был достигнут лимит подключений
  if (PauseIndicator == true) {
    resumeAccepting();  // Продолжаем прием запросов на подключение
    PauseIndicator = false;
  }
}

void PersoHost::on_ClientThreadDeleted_slot() {
  emit logging(QString("Клиентский поток удален. "));
}

void PersoHost::on_ClientConnectionDeleted_slot() {
  emit logging(QString("Клиентское соединение удалено. "));
}
