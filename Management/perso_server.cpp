#include <QHostAddress>

#include "client_connection.h"
#include "config.h"
#include "global_environment.h"
#include "log_system.h"
#include "perso_server.h"
#include "production_dispatcher.h"

PersoServer::PersoServer(const QString& name) : QTcpServer() {
  setObjectName(name);
  CurrentState = Idle;
  MaxNumberClientConnections = 0;

  // Загружаем настройки
  loadSettings();

  // Создаем идентификаторы для клиентов
  createClientIdentifiers();

  // Создаем систему выпуска транспондеров
  createDispatcherInstance();

  // Создаем таймер перезапуска
  createRestartTimer();

  connect(this, &PersoServer::logging,
          dynamic_cast<LogSystem*>(
              GlobalEnvironment::instance()->getObject("LogSystem")),
          &LogSystem::generate);
}

PersoServer::~PersoServer() {
  if (DispatcherThread->isRunning()) {
    DispatcherThread->quit();
    DispatcherThread->wait();
  }

  for (auto it = ClientThreads.begin(); it != ClientThreads.end(); ++it) {
    (*it)->quit();
    (*it)->wait();
  }
}

bool PersoServer::start() {
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

void PersoServer::sendLog(const QString& log) {
  emit logging(QString("%1 - %2").arg(objectName(), log));
}

void PersoServer::processCriticalError(const QString& log) {
  QString msg("Паника. Получена критическая ошибка. ");
  sendLog(msg + log);
#ifdef SERVER_PANIC_STATE_ENABLE
  CurrentState = Panic;
  deleteAllClientInstances();
#endif
}

void PersoServer::deleteAllClientInstances() {
  sendLog("Отключение всех клиентов.");
  for (auto it1 = Clients.begin(), it2 = Clients.end(); it1 != it2; ++it1) {
    deleteClientInstance(it1.key());
  }
}

void PersoServer::deleteClientInstance(int32_t id) {
  // Освобождаем занятый идентификатор
  FreeClientIds.push(id);

  // Удаляем отключившегося клиента и его поток из соответствующих реестров
  ClientThreads.value(id)->quit();
  if (!ClientThreads.value(id)->wait()) {
    processCriticalError(
        "Не удалось остановить поток отключившегося клиента. ");
  } else {
    sendLog(QString("Поток клиента %1 остановлен. ").arg(QString::number(id)));
  }
  ClientThreads.remove(id);
  Clients.remove(id);

  sendLog(QString("Клиент %1 удален из реестра. ").arg(QString::number(id)));
}

void PersoServer::createDispatcherInstance() {
  Dispatcher = std::unique_ptr<AbstractProductionDispatcher>(
      new ProductionDispatcher("ProductionDispatcher"));
  connect(this, &PersoServer::startProductionDispatcher_signal,
          Dispatcher.get(), &AbstractProductionDispatcher::start,
          Qt::BlockingQueuedConnection);
  connect(this, &PersoServer::stopProductionDispatcher_signal, Dispatcher.get(),
          &AbstractProductionDispatcher::stop, Qt::BlockingQueuedConnection);
  connect(Dispatcher.get(), &AbstractProductionDispatcher::errorDetected, this,
          &PersoServer::productionDispatcherErrorDetected,
          Qt::QueuedConnection);

  // Создаем отдельный поток для системы выпуска транспондеров
  DispatcherThread = std::unique_ptr<QThread>(new QThread());
  connect(DispatcherThread.get(), &QThread::started, Dispatcher.get(),
          &AbstractProductionDispatcher::onInstanceThreadStarted);

  // Запускаем поток
  Dispatcher->moveToThread(DispatcherThread.get());
  DispatcherThread->start();
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
  QString clientName = QString("client%1").arg(QString::number(clientId));

  // Создаем новое клиент-подключение
  std::shared_ptr<AbstractClientConnection> newClient(
      new ClientConnection(clientName, clientId, socketDescriptor));

  connect(newClient.get(), &AbstractClientConnection::disconnected, this,
          &PersoServer::clientDisconnected_slot);

  // Добавляем клиента в реестр
  Clients.insert(clientId, newClient);
  sendLog(QString("Новый клиент создан и зарегистрирован в реестре с "
                  "идентификатором %1. ")
              .arg(QString::number(newClient->id())));

  // Создаем отдельный поток для клиента
  std::shared_ptr<QThread> newClientThread(new QThread());
  newClient->moveToThread(newClientThread.get());

  connect(newClientThread.get(), &QThread::started, newClient.get(),
          &AbstractClientConnection::onInstanceThreadStarted);
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
  // Удаляем клиента
  int32_t clientId = disconnectedClient->id();
  deleteClientInstance(clientId);

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

void PersoServer::productionDispatcherErrorDetected(ReturnStatus status) {
  processCriticalError(
      QString("Диспетчер производства детектировал ошибку: %1.")
          .arg(QString::number(static_cast<size_t>(status))));
}
