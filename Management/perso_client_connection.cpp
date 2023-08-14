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

void PersoClientConnection::instanceTesting() {
  if (thread() != QApplication::instance()->thread())
    emit logging("Отдельный поток выделен. ");
  else
    emit logging("Отдельный поток не выделен. ");
}

void PersoClientConnection::processingReceivedRawData() {
  QJsonParseError status;
  CurrentCommand = QJsonDocument::fromJson(ReceivedRawData, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    emit logging("Ошибка парсинга JSON команды. ");
    return;
  }

  // Выделяем список пар ключ-значение из JSON-файла
  QJsonObject CommandObject = CurrentCommand.object();

  // Синтаксическая проверка
  if (CommandObject.value("CommandName") != QJsonValue::Undefined) {
    emit logging(
        "Обнаружена синтаксическая ошибка: в запросе отсутствует название "
        "команды. ");
    return;
  }

  // Вызываем соответствующий обработчик команды
  if (CommandObject.value("CommandName").toString() == "EchoRequest") {
    echoRequestProcessing(&CommandObject);
  } else {
    emit logging(
        "Обнаружена синтаксическая ошибка: получена неизвестная команда. ");
  }

  // Отправляем ответ на команду
  transmitResponseRawData();
}

void PersoClientConnection::transmitResponseRawData() {
  QByteArray transmittedRawData;
  QDataStream serializator(transmittedRawData);
  serializator.setVersion(QDataStream::Qt_5_12);

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << CurrentResponse;
  serializator.device()->seek(0);
  serializator << uint32_t(transmittedRawData.size() - sizeof(uint32_t));

  // Отправляем сформируем блок данных
  Socket->write(transmittedRawData);
}

void PersoClientConnection::echoRequestProcessing(QJsonObject* commandJson) {
  CurrentResponse.setObject(QJsonObject());
  QJsonObject responseJson = CurrentResponse.object();

  // Заголовок ответа на команду
  responseJson["CommandName"] = "EchoResponse";

  if (commandJson->value("EchoData") != QJsonValue::Undefined) {
    responseJson["EchoData"] = commandJson->value("EchoData");
  } else {
    emit logging(
        "Обнаружена синтаксическая ошибка в команде EchoRequest: отсутствуют "
        "эхо-данные. ");
  }

  Socket->write(ReceivedRawData);
}

void PersoClientConnection::getFirmwareProcessing(QJsonObject* json) {}

void PersoClientConnection::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres - " + log);
  else
    emit logging("Unknown - " + log);
}

void PersoClientConnection::on_SocketReadyRead_slot() {
  uint32_t blockSize = 0;              // Размер блока
  QDataStream deserializator(Socket);  // Дессериализатор
  deserializator.setVersion(
      QDataStream::Qt_5_12);  // Настраиваем версию сериализатора

  while (true) {
    // Если блок данных еще не начал формироваться
    if (!blockSize) {
      // Если размер поступивших байт меньше размера поля с размером байт, то
      // блок поступившие данные отбрасываются
      if (Socket->bytesAvailable() < sizeof(uint32_t)) {
        break;
      }
      deserializator >> blockSize;

      emit logging(QString("Размер полученного блока данных: %1.")
                       .arg(QString::number(blockSize)));
    }

    // Дожидаемся пока весь блок данных придет целиком
    if (Socket->bytesAvailable() < blockSize) {
      emit logging(
          "Блок получен не целиком. Ожидается прием следующих частей. ");
      break;
    }

    // Если блок был получен целиком, то осуществляем его дессериализацию
    deserializator >> ReceivedRawData;
    emit logging("Блок полученных данных: " + ReceivedRawData);

    // Выходим
    break;
  }

  // Осуществляем обработку полученных данных
  processingReceivedRawData();
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
