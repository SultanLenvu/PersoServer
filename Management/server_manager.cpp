#include "server_manager.h"

/* Диспетчер DSRC  */
//==================================================================================
ServerManager::ServerManager(QObject* parent) : QObject(parent) {
  setObjectName("ServerManager");

  // Буфер для отображения данных базы данных
  Buffer = new DatabaseBuffer(this);

  // Загружаем пользовательские настройки
  UserSettings = new QSettings(COMPANY_NAME, PROGRAM_NAME);

  // Создаем среду выполнения для потока сервера
  createServerInstance();

  // Запускаем поток сервера
  ServerThread->start();
}

ServerManager::~ServerManager() {
  ServerThread->quit();
  ServerThread->wait();
}

DatabaseBuffer* ServerManager::buffer() {
  return Buffer;
}

void ServerManager::start() {
  if ((Host) && (Host->isListening())) {
    emit logging("Сервер уже запущен. ");
    return;
  }

  emit logging("Запуск сервера персонализации. ");
  emit serverStart_signal();
}

void ServerManager::stop() {
  emit logging("Остановка сервера персонализации. ");
  emit serverStop_signal();
}

void ServerManager::showProductionLines() {
  Buffer->clear();

  emit logging("Представление линий производства. ");
  emit getProductionLines_signal(Buffer);
}

void ServerManager::showTransponders() {
  Buffer->clear();

  emit logging("Представление транспондеров. ");
  emit getTransponders_signal(Buffer);
}

void ServerManager::showOrders() {
  Buffer->clear();

  emit logging("Представление заказов. ");
  emit getOrders_signal(Buffer);
}

void ServerManager::showIssuers() {
  Buffer->clear();

  emit logging("Представление заказчиков. ");
  emit getIssuers_signal(Buffer);
}

void ServerManager::showBoxes() {
  Buffer->clear();

  emit logging("Представление боксов. ");
  emit getBoxes_signal(Buffer);
}

void ServerManager::showPallets() {
  Buffer->clear();

  emit logging("Представление палет. ");
  emit getPallets_signal(Buffer);
}

void ServerManager::showCustomResponse(const QString& req) {
  Buffer->clear();

  emit logging("Представление ответа на кастомный запрос. ");
  emit getCustomResponse_signal(req, Buffer);
}

void ServerManager::createServerInstance() {
  // Создаем сервер и поток для него
  Host = new PersoHost(nullptr, UserSettings);
  ServerThread = new QThread(this);

  // Переносим сервер в поток
  Host->moveToThread(ServerThread);

  // Подключаем логгирование к серверу
  connect(Host, &PersoHost::logging, this, &ServerManager::proxyLogging);
  // Когда поток завершит работу, Server будет удален
  connect(ServerThread, &QThread::finished, Host, &QObject::deleteLater);
  // Когда поток завершит работу, он будет удален
  connect(ServerThread, &QThread::finished, ServerThread,
          &QObject::deleteLater);
  // Когда поток завершит работу, вызываем метод обработки
  connect(ServerThread, &QThread::finished, this,
          &ServerManager::serverThreadFinished);

  // Подключаем функционал
  connect(this, &ServerManager::serverStart_signal, Host, &PersoHost::start);
  connect(this, &ServerManager::serverStop_signal, Host, &PersoHost::stop);
  connect(this, &ServerManager::getProductionLines_signal, Host,
          &PersoHost::getProductionLines);
  connect(this, &ServerManager::getTransponders_signal, Host,
          &PersoHost::getTransponders);
  connect(this, &ServerManager::getOrders_signal, Host, &PersoHost::getOrders);
  connect(this, &ServerManager::getIssuers_signal, Host,
          &PersoHost::getIssuers);
  connect(this, &ServerManager::getBoxes_signal, Host, &PersoHost::getBoxes);
  connect(this, &ServerManager::getPallets_signal, Host,
          &PersoHost::getPallets);
  connect(this, &ServerManager::getCustomResponse_signal, Host,
          &PersoHost::getCustomResponse);
}

/*
 * Приватные слоты
 */

void ServerManager::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("PostgresController"))
    emit logging(QString("Postgres сontroller - ") + log);
  else if (sender()->objectName() == QString("DatabaseController"))
    emit logging(QString("Postgres сontroller - ") + log);
  else if (sender()->objectName() == QString("DatabaseBuffer"))
    emit logging(QString("Database buffer - ") + log);
  else if (sender()->objectName() == QString("PersoHost"))
    emit logging(QString("Perso server - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void ServerManager::serverThreadFinished() {
  emit logging("Поток сервера завершился. ");
  Host = nullptr;
  ServerThread = nullptr;
}

//==================================================================================
