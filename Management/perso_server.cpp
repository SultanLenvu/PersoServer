#include <QHostAddress>

#include "ClientConnection/client_connection.h"
#include "Log/log_system.h"
#include "ProductionDispatcher/general_production_dispatcher.h"
#include "perso_server.h"

PersoServer::PersoServer(const QString& name) : QTcpServer() {
  setObjectName(name);
  CurrentState = Idle;
  MaxNumberClientConnections = 0;

  // Загружаем настройки
  loadSettings();

  // Создаем идентификаторы для клиентов
  createClientIdentifiers();

  // Создаем систему выпуска транспондеров
  createProductionDispatcherInstance();

  // Создаем таймер перезапуска
  createRestartTimer();
}

PersoServer::~PersoServer() {
  if (ProductionDispatcherThread->isRunning()) {
    ProductionDispatcherThread->quit();
    ProductionDispatcherThread->wait();
  }

  for (QHash<size_t, std::unique_ptr<QThread>>::iterator it =
           ClientThreads.begin();
       it != ClientThreads.end(); it++) {
    (*it)->exit();
    (*it)->wait();
  }
}

bool PersoServer::start() {
  sendLog("Проверка конфигурации");
  if (!checkConfiguration()) {
    sendLog("Проверка конфигурации провалена. Запуск сервера невозможен.");
    RestartTimer->start();
    return false;
  }

  // Запускаем диспетчер производства
  ReturnStatus ret;
  emit startProductionDispatcher_signal(ret);
  if (ret != ReturnStatus::NoError) {
    sendLog(
        "Не удалось запустить систему выпуска транспондеров. Запуск сервера "
        "невозможен.");
    RestartTimer->start();
    return false;
  }

  // Поднимаем сервер
  sendLog(
      QString("Попытка запуска на %1:%2.")
          .arg(ListeningAddress.toString(), QString::number(ListeningPort)));
  if (!listen(ListeningAddress, ListeningPort)) {
    sendLog("Не удалось запуститься. ");
    RestartTimer->start();
    return false;
  }
  // Если сервер поднялся
  if (thread() == QCoreApplication::instance()->thread()) {
    sendLog("Запущен в главном потоке. ");
  } else {
    sendLog("Запущен в отдельном потоке. ");
  }

  // Изменяем состояние
  CurrentState = Work;
  return true;
}

void PersoServer::stop() {
  // Останавливаем сервер
  close();
  sendLog("Остановлен. ");

  // Останавливаем систему выпуска транспондеров
  emit stopProductionDispatcher_signal();

  CurrentState = Idle;
}

void PersoServer::incomingConnection(qintptr socketDescriptor) {
  sendLog("Получен запрос на новое подключение. ");

  if (CurrentState == Panic) {
    sendLog(
        "В процессе функционирования получена критическая ошибка. Обработка "
        "новых клиентских подключений невозможна.");
    return;
  }

  // Если свободных идентификаторов нет
  if (FreeClientIds.isEmpty()) {
    pauseAccepting();  // Приостанавливаем прием новых подключений
    CurrentState = Paused;

    sendLog("Достигнут лимит подключений, прием новых приостановлен. ");
    return;
  }

  // Создаем среду выполнения для клиента
  createClientInstance(socketDescriptor);
}

void PersoServer::loadSettings() {
  QSettings settings;

  RestartPeriod = settings.value("perso_server/restart_period").toInt();
  MaxNumberClientConnections =
      settings.value("perso_server/max_number_client_connection").toInt();

  ListeningAddress =
      QHostAddress(settings.value("perso_server/listen_ip").toString());
  ListeningPort = settings.value("perso_server/listen_port").toInt();
}

void PersoServer::sendLog(const QString& log) const {
  LogSystem::instance()->generate(objectName() + " - " + log);
}

void PersoServer::processCriticalError(const QString& log) {
  QString msg("Паника. Получена критическая ошибка. ");
  sendLog(msg + log);
  //  CurrentState = Panic;
}

bool PersoServer::checkConfiguration() {
  return ProductionDispatcher->checkConfiguration();
}

void PersoServer::createProductionDispatcherInstance() {
  ProductionDispatcher = std::unique_ptr<AbstractProductionDispatcher>(
      new GeneralProductionDispatcher("GeneralProductionDispatcher"));
  connect(this, &PersoServer::startProductionDispatcher_signal,
          ProductionDispatcher.get(), &AbstractProductionDispatcher::start,
          Qt::BlockingQueuedConnection);
  connect(this, &PersoServer::stopProductionDispatcher_signal,
          ProductionDispatcher.get(), &AbstractProductionDispatcher::stop,
          Qt::BlockingQueuedConnection);
  connect(ProductionDispatcher.get(), &AbstractProductionDispatcher::logging,
          LogSystem::instance(), &LogSystem::generate);
  connect(ProductionDispatcher.get(),
          &AbstractProductionDispatcher::errorDetected, this,
          &PersoServer::productionDispatcherErrorDetected,
          Qt::BlockingQueuedConnection);

  // Создаем отдельный поток для системы выпуска транспондеров
  ProductionDispatcherThread = std::unique_ptr<QThread>(new QThread());
  ProductionDispatcher->moveToThread(ProductionDispatcherThread.get());

  // Запускаем поток
  ProductionDispatcherThread->start();
}

void PersoServer::createClientIdentifiers() {
  FreeClientIds.clear();
  for (int32_t i = 1; i <= MaxNumberClientConnections; i++) {
    FreeClientIds.push(i);
  }
}

void PersoServer::createClientInstance(qintptr socketDescriptor) {
  // Выделяем свободный идентификатор
  int32_t clientId = FreeClientIds.pop();
  QString clientName =
      std::move(QString("client%1").arg(QString::number(clientId)));

  // Создаем новое клиент-подключение
  std::unique_ptr<AbstractClientConnection> newClient =
      std::unique_ptr<AbstractClientConnection>(
          new ClientConnection(clientName, clientId, socketDescriptor));

  connect(newClient.get(), &AbstractClientConnection::logging,
          LogSystem::instance(), &LogSystem::generate);
  connect(newClient.get(), &AbstractClientConnection::disconnected, this,
          &PersoServer::clientDisconnected_slot);

  // Добавляем клиента в реестр
  Clients.insert(clientId, newClient);
  sendLog(QString("Новый клиент создан и зарегистрирован в реестре с "
                  "идентификатором %1. ")
              .arg(QString::number(newClient->getId())));

  // Создаем отдельный поток для клиента
  std::unique_ptr<QThread> newClientThread =
      std::make_unique<QThread>(new QThread());
  newClient->moveToThread(newClientThread.get());

  connect(newClientThread.get(), &QThread::destroyed, this,
          &PersoServer::clientThreadDeleted_slot);

  // Добавляем поток в соответствующий реестр и запускаем
  ClientThreads.insert(clientId, newClientThread);
  newClientThread->start();

  sendLog("Клиентский поток запущен. ");
}

void PersoServer::createRestartTimer() {
  RestartTimer = std::make_unique<QTimer>(new QTimer());
  RestartTimer->setInterval(RestartPeriod * 1000);

  connect(RestartTimer.get(), &QTimer::timeout, this,
          &PersoServer::restartTimerTimeout_slot);
}

void PersoServer::clientDisconnected_slot() {
  ClientConnection* disconnectedClient =
      dynamic_cast<ClientConnection*>(sender());
  if (!disconnectedClient) {
    processCriticalError(
        "Не удалось получить доступ к данным отключившегося клиента. ");
    return;
  }
  // Освобождаем занятый идентификатор
  uint32_t clientId = disconnectedClient->getId();
  FreeClientIds.push(clientId);

  // Удаляем отключившегося клиента и его поток из соответствующих реестров
  disconnectedClient->thread()->quit();
  if (!disconnectedClient->thread()->wait()) {
    processCriticalError(
        "Не удалось остановить поток отключившегося клиента. ");
  } else {
    sendLog(QString("Поток клиента %1 остановлен. ")
                .arg(QString::number(clientId)));
  }
  ClientThreads.remove(clientId);
  Clients.remove(clientId);

  sendLog(
      QString("Клиент %1 удален из реестра. ").arg(QString::number(clientId)));

  // Если ранее был достигнут лимит подключений
  if (CurrentState == Paused) {
    resumeAccepting();  // Продолжаем прием запросов на подключение
    CurrentState = Work;
  }
}

void PersoServer::clientThreadDeleted_slot() {
  sendLog(QString("Клиентский поток удален. "));
}

void PersoServer::restartTimerTimeout_slot() {
  if (start()) {
    RestartTimer->stop();
  }
}

void PersoServer::productionDispatcherErrorDetected(ReturnStatus& status) {}
