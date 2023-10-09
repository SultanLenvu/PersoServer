#include "perso_server.h"

PersoServer::PersoServer(QObject* parent) : QTcpServer(parent) {
  setObjectName("PersoServer");
  CurrentState = Idle;
  MaxNumberClientConnections = 0;

  // Загружаем настройки
  loadSettings();

  // Создаем идентификаторы для клиентов
  createClientIdentifiers();

  // Создаем систему выпуска транспондеров
  createReleaserInstance();
}

PersoServer::~PersoServer() {
  if (ReleaserThread->isRunning()) {
    ReleaserThread->quit();
    ReleaserThread->wait();
  }

  QMap<int32_t, QThread*>::iterator it1;
  for (it1 = ClientThreads.begin(); it1 != ClientThreads.end(); it1++) {
    delete it1.value();
  }

  QMap<int32_t, PersoClientConnection*>::iterator it2;
  for (it2 = Clients.begin(); it2 != Clients.end(); it2++) {
    delete it2.value();
  }
}

bool PersoServer::start() {
  // Запускаем систему выпуска транспондеров
  TransponderReleaseSystem::ReturnStatus status;
  emit startReleaser_signal(&status);

  // Поднимаем сервер
  emit logging(
      QString("Попытка запуска на %1:%2.")
          .arg(ListeningAddress.toString(), QString::number(ListeningPort)));
  if (!listen(ListeningAddress, ListeningPort)) {
    emit logging("Не удалось запуститься. ");
    return false;
  }

  // Если сервер поднялся
  if (thread() == QCoreApplication::instance()->thread()) {
    emit logging("Запущен в главном потоке. ");
  } else {
    emit logging("Запущен в отдельном потоке. ");
  }

  // Изменяем состояние
  CurrentState = Work;
  return true;
}

void PersoServer::stop() {
  // Останавливаем релизер
  Releaser->stop();

  // Останавливаем сервер
  close();
  emit logging("Остановлен. ");

  // Останавливаем систему выпуска транспондеров
  emit stopReleaser_signal();

  CurrentState = Idle;
}

void PersoServer::incomingConnection(qintptr socketDescriptor) {
  emit logging("Получен запрос на новое подключение. ");

  // Если свободных идентификаторов нет
  if (FreeClientIds.size() == 0) {
    pauseAccepting();  // Приостанавливаем прием новых подключений
    CurrentState = Paused;

    emit logging("Достигнут лимит подключений, прием новых приостановлен. ");
    return;
  }

  // Создаем среду выполнения для клиента
  createClientInstance(socketDescriptor);

  // Проверяем созданную среду выполнения
  emit checkNewClientInstance();
}

void PersoServer::loadSettings() {
  QSettings settings;

  MaxNumberClientConnections =
      settings.value("Server/MaxNumberClientConnection").toInt();

  ListeningAddress = QHostAddress(settings.value("Server/ListenIp").toString());
  if (ListeningAddress.isNull()) {
    ListeningAddress = QHostAddress(PERSO_SERVER_DEFAULT_IP);
  }

  ListeningPort = settings.value("Server/ListenPort").toInt();
  if (ListeningPort == 0) {
    ListeningPort = PERSO_SERVER_DEFAULT_PORT;
  }
}

void PersoServer::createReleaserInstance() {
  Releaser = new TransponderReleaseSystem(nullptr);
  connect(Releaser, &TransponderReleaseSystem::logging, this,
          &PersoServer::proxyLogging);
  connect(this, &PersoServer::startReleaser_signal, Releaser,
          &TransponderReleaseSystem::start);
  connect(this, &PersoServer::stopReleaser_signal, Releaser,
          &TransponderReleaseSystem::stop);

  // Создаем отдельный поток для системы выпуска транспондеров
  ReleaserThread = new QThread(this);
  Releaser->moveToThread(ReleaserThread);

  connect(ReleaserThread, &QThread::finished, ReleaserThread,
          &QThread::deleteLater);
  connect(ReleaserThread, &QThread::finished, Releaser,
          &PersoClientConnection::deleteLater);

  // Запускаем поток
  ReleaserThread->start();
}

void PersoServer::createClientIdentifiers() {
  FreeClientIds.clear();
  for (int32_t i = 1; i < MaxNumberClientConnections; i++) {
    FreeClientIds.insert(i);
  }
}

void PersoServer::createClientInstance(qintptr socketDescriptor) {
  // Выделяем свободный идентификатор
  int32_t clientId = *FreeClientIds.constBegin();
  FreeClientIds.remove(clientId);

  // Создаем новое клиент-подключение
  PersoClientConnection* newClient =
      new PersoClientConnection(clientId, socketDescriptor);

  connect(newClient, &PersoClientConnection::logging, this,
          &PersoServer::proxyLogging);
  connect(newClient, &PersoClientConnection::disconnected, this,
          &PersoServer::on_ClientDisconnected_slot);
  connect(this, &PersoServer::checkNewClientInstance, newClient,
          &PersoClientConnection::instanceTesting);

  // Добавляем клиента в реестр
  Clients.insert(clientId, newClient);
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
  ClientThreads.insert(clientId, newClientThread);

  // Соединяем клиента с системой выпуска транспондеров
  connect(newClient, &PersoClientConnection::releaserAuthorize_signal, Releaser,
          &TransponderReleaseSystem::authorize);
  connect(newClient, &PersoClientConnection::releaseRelease_signal, Releaser,
          &TransponderReleaseSystem::release);
  connect(newClient, &PersoClientConnection::releaserConfirmRelease_signal, Releaser,
          &TransponderReleaseSystem::confirmRelease);
  connect(newClient, &PersoClientConnection::releaserRerelease_signal, Releaser,
          &TransponderReleaseSystem::rerelease);
  connect(newClient, &PersoClientConnection::releaserConfirmRerelease_signal, Releaser,
          &TransponderReleaseSystem::confirmRerelease);
  connect(newClient, &PersoClientConnection::releaserSearch_signal, Releaser,
          &TransponderReleaseSystem::search);

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
  else if (sender()->objectName() == "TransponderReleaseSystem")
    emit logging("Releaser - " + log);
  else
    emit logging("Unknown - " + log);
}

void PersoServer::on_ClientDisconnected_slot() {
  // Освобождаем занятый идентификатор
  uint32_t clientId = dynamic_cast<PersoClientConnection*>(sender())->getId();
  FreeClientIds.insert(clientId);

  // Удаляем отключившегося клиента и его поток из соответствующих реестров
  ClientThreads.remove(clientId);
  Clients.remove(clientId);

  emit logging(
      QString("Клиент %1 удален из реестра. ").arg(QString::number(clientId)));

  // Если ранее был достигнут лимит подключений
  if (CurrentState == Paused) {
    resumeAccepting();  // Продолжаем прием запросов на подключение
    CurrentState = Work;
  }
}

void PersoServer::on_ClientThreadDeleted_slot() {
  emit logging(QString("Клиентский поток удален. "));
}

void PersoServer::on_ClientConnectionDeleted_slot() {
  emit logging(QString("Клиентское соединение удалено. "));
}
