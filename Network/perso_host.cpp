#include "perso_host.h"

PersoHost::PersoHost(QObject* parent) : QTcpServer(parent) {
  setObjectName("PersoHost");
  PauseIndicator = false;
  MaxNumberClientConnections = 0;

  // Загружаем настройки
  loadSettings();

  // Создаем идентификаторы для клиентов
  createClientIdentifiers();

  // Интерфейс для централизованного доступа к базе данных
  Database = new PostgresController(this, QString("ServerConnection"));
  connect(Database, &IDatabaseController::logging, this,
          &PersoHost::proxyLogging);

  // Настраиваем контроллер базы данных
  Database->applySettings();
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

void PersoHost::incomingConnection(qintptr socketDescriptor) {
  emit logging("Получен запрос на новое подключение. ");

  // Если свободных идентификаторов нет
  if (FreeClientIds.size() == 0) {
    pauseAccepting();  // Блокируем прием новых подключений
    PauseIndicator = true;

    emit logging("Достигнут лимит подключений, прием новых приостановлен. ");
    return;
  }

  // Создаем среду выполнения для клиента
  createClientInstance(socketDescriptor);

  // Проверяем созданную среду выполнения
  emit checkNewClientInstance();
}

void PersoHost::loadSettings() {
  QSettings settings;

  MaxNumberClientConnections =
      settings.value("PersoHost/MaxNumberClientConnection").toInt();
}

void PersoHost::createClientIdentifiers() {
  FreeClientIds.clear();
  for (int32_t i = 1; i < MaxNumberClientConnections; i++) {
    FreeClientIds.insert(i);
  }
}

void PersoHost::applySettings() {
  emit logging("Применение новых настроек. ");

  loadSettings();
  createClientIdentifiers();

  Database->applySettings();
}

void PersoHost::createClientInstance(qintptr socketDescriptor) {
  // Выделяем свободный идентификатор
  int32_t clientId = *FreeClientIds.constBegin();
  FreeClientIds.remove(clientId);

  // Создаем новое клиент-подключение
  PersoClientConnection* newClient =
      new PersoClientConnection(clientId, socketDescriptor);

  connect(newClient, &PersoClientConnection::logging, this,
          &PersoHost::proxyLogging);
  connect(newClient, &PersoClientConnection::disconnected, this,
          &PersoHost::on_ClientDisconnected_slot);
  connect(this, &PersoHost::checkNewClientInstance, newClient,
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
          &PersoHost::on_ClientConnectionDeleted_slot);
  connect(newClientThread, &QThread::destroyed, this,
          &PersoHost::on_ClientThreadDeleted_slot);

  // Добавляем поток в соответствующий реестр
  ClientThreads.insert(clientId, newClientThread);

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
  // Освобождаем занятый идентификатор
  uint32_t clientId = dynamic_cast<PersoClientConnection*>(sender())->getId();
  FreeClientIds.insert(clientId);

  // Удаляем отключившегося клиента и его поток из соответствующих реестров
  ClientThreads.remove(clientId);
  Clients.remove(clientId);

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
