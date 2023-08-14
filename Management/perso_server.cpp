#include "perso_server.h"

PersoServer::PersoServer(QObject* parent, QSettings* settings)
    : QTcpServer(parent) {
  setObjectName("PersoServer");

  Settings = settings;

  PauseIndicator = false;

  // Интерфейс для централизованного доступа к базе данных
  Database = new PostgresController(this, QString("ServerConnection"));
  connect(Database, &DatabaseControllerInterface::logging, this,
          &PersoServer::proxyLogging);
  // Настраиваем контроллер базы данных
  Database->applySettings(Settings);
}

PersoServer::~PersoServer() {
  emit logging("Уничтожен. ");
}

void PersoServer::start() {
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

void PersoServer::stop() {
  close();
  emit logging("Остановлен. ");
}

void PersoServer::incomingConnection(qintptr socketDescriptor) {
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

void PersoServer::createClientInstance(qintptr socketDescriptor) {
  // Создаем новое клиент-подключение
  PersoClientConnection* newClient =
      new PersoClientConnection(Clients.size(), socketDescriptor, Settings);

  connect(newClient, &PersoClientConnection::logging, this,
          &PersoServer::proxyLogging);
  connect(newClient, &PersoClientConnection::disconnected, this,
          &PersoServer::on_ClientDisconnected_slot);
  connect(this, &PersoServer::checkNewClientInstance, newClient,
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
          &PersoServer::on_ClientConnectionDeleted_slot);
  connect(newClientThread, &QThread::destroyed, this,
          &PersoServer::on_ClientThreadDeleted_slot);

  // Добавляем поток в соответствующий реестр
  ClientThreads.append(newClientThread);

  // Запускаем поток
  newClientThread->start();
  emit logging("Клиентский поток запущен. ");
}

void PersoServer::proxyLogging(const QString& log) {
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

void PersoServer::on_ClientDisconnected_slot() {
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

void PersoServer::on_ClientThreadDeleted_slot() {
  emit logging(QString("Клиентский поток удален. "));
}

void PersoServer::on_ClientConnectionDeleted_slot() {
  emit logging(QString("Клиентское соединение удалено. "));
}
