#include "perso_client_connection.h"

PersoClientConnection::PersoClientConnection(uint32_t id,
                                             qintptr socketDescriptor,
                                             QSettings* settings) {
  setObjectName("PersoClientConnection");

  // Идентификатор клиента
  ID = id;

  // Дескриптор системного сокета
  SocketDescriptor = socketDescriptor;

  // Запоминаем настройки
  Settings = settings;

  // Пока поток клиента не запущен
  // Сокет клиента
  Socket = new QTcpSocket(this);
  Socket->setSocketDescriptor(SocketDescriptor);
  connect(Socket, &QTcpSocket::readyRead, this,
          &PersoClientConnection::on_SocketReadyRead_slot);
  connect(Socket, &QTcpSocket::disconnected, this,
          &PersoClientConnection::on_SocketDisconnected_slot);
  connect(Socket, &QTcpSocket::disconnected, Socket, &QTcpSocket::deleteLater);

  // Интерфейс для доступа к базе данных
  Database = new PostgresController(
      this, QString("Client %1 database connection").arg(QString::number(ID)));
  connect(Database, &DatabaseControllerInterface::logging, this,
          &PersoClientConnection::proxyLogging);
  // Настраиваем контроллер базы данных
  Database->applySettings(Settings);
  Database->connect();

  // Таймер для отсчета времени экспирации
  ExpirationTimer = new QTimer(this);
  ExpirationTimer->setInterval(CLIENT_CONNECTION_MAX_DURATION);
  connect(ExpirationTimer, &QTimer::timeout, this,
          &PersoClientConnection::on_ExpirationTimerTimeout_slot);
  ExpirationTimer->start();
}

PersoClientConnection::~PersoClientConnection() {}

uint32_t PersoClientConnection::getId() {
  return ID;
}

void PersoClientConnection::startInctance() {
  emit logging("Среда выполнения запущена. ");
}

void PersoClientConnection::instanceTesting() {
  if (thread() != QApplication::instance()->thread())
    emit logging("Отдельный поток выделен. ");
  else
    emit logging("Отдельный поток не выделен. ");
}

void PersoClientConnection::echoRequestProcessing() {
  Socket->write(ReceivedData);
}

void PersoClientConnection::getFirmwareProcessing() {}

void PersoClientConnection::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres - " + log);
  else
    emit logging("Unknown - " + log);
}

void PersoClientConnection::on_SocketReadyRead_slot() {
  ReceivedData = Socket->readAll();

  emit logging("Получены данные: " + ReceivedData);

  echoRequestProcessing();
}

void PersoClientConnection::on_SocketDisconnected_slot() {
  ExpirationTimer->stop();
  emit logging("Отключился. ");

  //Отправляем сигнал об отключении клиента
  emit disconnected();
}

void PersoClientConnection::on_ExpirationTimerTimeout_slot() {
  ExpirationTimer->stop();
  emit logging("Экспирация времени подключения. ");

  // Закрываем соединение
  Socket->close();
}
