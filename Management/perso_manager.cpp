#include "perso_manager.h"

/* Диспетчер DSRC  */
//==================================================================================
PersoManager::PersoManager(QObject* parent) : QObject(parent) {
  setObjectName("PersoManager");

  DatabaseController = new PostgresController(this, "ManagerConnection");
  connect(DatabaseController, &DatabaseControllerInterface::logging, this,
          &PersoManager::proxyLogging);

  Buffer = new DatabaseBuffer(this);
  connect(Buffer, &DatabaseBuffer::logging, this, &PersoManager::proxyLogging);

  // Загружаем пользовательские настройки
  UserSettings = new QSettings(COMPANY_NAME, PROGRAM_NAME);

  Server = nullptr;
  ServerThread = nullptr;

  // Создаем среду выполнения для потока сервера
  createServerInstance();

  // Запускаем поток сервера
  ServerThread->start();
}

PersoManager::~PersoManager() {
  ServerThread->quit();
  ServerThread->wait();
}

DatabaseBuffer* PersoManager::buffer() {
  return Buffer;
}

void PersoManager::startServer() {
  if ((Server) && (Server->isListening())) {
    emit logging("Сервер уже запущен. ");
    return;
  }

  emit logging("Запуск сервера персонализации. ");
  emit serverStart_request();
}

void PersoManager::stopServer() {
  emit logging("Остановка сервера персонализации. ");
  emit serverStop_request();
}

void PersoManager::connectDatabase() {
  DatabaseController->connect();
}

void PersoManager::disconnectDatabase() {
  DatabaseController->disconnect();
}

void PersoManager::performCustomSqlRequest(const QString& req) {
  Buffer->clear();

  DatabaseController->execCustomRequest(req, Buffer);

  Buffer->log();
}

void PersoManager::userSettings(void) {}

void PersoManager::createServerInstance() {
  // Создаем сервер и поток для него
  Server = new PersoServer(nullptr, UserSettings);
  ServerThread = new QThread(this);

  // Переносим сервер в поток
  Server->moveToThread(ServerThread);

  // Подключаем логгирование к серверу
  connect(Server, &PersoServer::logging, this, &PersoManager::proxyLogging);
  // Когда поток завершит работу, Server будет удален
  connect(ServerThread, &QThread::finished, Server, &QObject::deleteLater);
  // Когда поток завершит работу, он будет удален
  connect(ServerThread, &QThread::finished, ServerThread,
          &QObject::deleteLater);
  // Когда поток завершит работу, вызываем метод обработки
  connect(ServerThread, &QThread::finished, this,
          &PersoManager::serverThreadFinished);
  // Запускаем сервер по сигналу
  connect(this, &PersoManager::serverStart_request, Server,
          &PersoServer::start);
  // Останавливаем сервер по сигналу
  connect(this, &PersoManager::serverStop_request, Server, &PersoServer::stop);
}

/*
 * Приватные слоты
 */

void PersoManager::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("PostgresController"))
    emit logging(QString("Postgres сontroller - ") + log);
  else if (sender()->objectName() == QString("DatabaseController"))
    emit logging(QString("Postgres сontroller - ") + log);
  else if (sender()->objectName() == QString("DatabaseBuffer"))
    emit logging(QString("Database buffer - ") + log);
  else if (sender()->objectName() == QString("PersoServer"))
    emit logging(QString("Perso server - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void PersoManager::serverThreadFinished() {
  emit logging("Поток сервера завершился. ");
  Server = nullptr;
  ServerThread = nullptr;
}

//==================================================================================
