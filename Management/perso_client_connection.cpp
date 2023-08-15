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

void PersoClientConnection::processingReceivedDataBlock(QByteArray* dataBlock) {
  QJsonParseError status;
  CurrentCommand = QJsonDocument::fromJson(*dataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    emit logging("Ошибка парсинга JSON команды. ");
    return;
  } else {
    emit logging("Обработка полученного блока данных. ");
  }

  // Выделяем список пар ключ-значение из JSON-файла
  QJsonObject CommandObject = CurrentCommand.object();

  // Синтаксическая проверка
  if (CommandObject.value("CommandName") == QJsonValue::Undefined) {
    emit logging(
        "Обнаружена синтаксическая ошибка: отсутствует название команды. ");
    return;
  }

  // Вызываем соответствующий обработчик команды
  if (CommandObject.value("CommandName").toString() == "EchoRequest") {
    processingEchoRequest(&CommandObject);
  } else if (CommandObject.value("CommandName").toString() ==
             "FirmwareRequest") {
    processingFirmwareRequest(&CommandObject);
  } else {
    emit logging(
        "Обнаружена синтаксическая ошибка: получена неизвестная команда. ");
    return;
  }

  // Отправляем ответ на команду
  transmitSerializedData();
}

void PersoClientConnection::transmitSerializedData() {
  QByteArray serializedData;
  QDataStream serializator(&serializedData, QIODevice::WriteOnly);
  serializator.setVersion(QDataStream::Qt_5_12);

  emit logging("Отправляемый блок данных: " +
               CurrentResponse.toJson(QJsonDocument::Compact));
  emit logging(
      QString::number(CurrentResponse.toJson(QJsonDocument::Compact).size()));

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << CurrentResponse.toJson(QJsonDocument::Compact);
  serializator.device()->seek(0);
  serializator << uint32_t(serializedData.size() - sizeof(uint32_t));

  emit logging(
      QString::number(uint32_t(serializedData.size() - sizeof(uint32_t))));

  // Отправляем сформируем блок данных
  Socket->write(serializedData);
}

void PersoClientConnection::processingEchoRequest(QJsonObject* commandJson) {
  emit logging("Выполнение команды EchoRequest. ");

  // Синтаксическая проверка
  if (commandJson->value("EchoData") == QJsonValue::Undefined) {
    emit logging(
        "Обнаружена синтаксическая ошибка в команде EchoRequest: отсутствуют "
        "эхо-данные. ");
    return;
  }

  QJsonObject responseJson;

  // Заголовок ответа на команду
  responseJson["CommandName"] = "EchoResponse";

  // Данные
  responseJson["EchoData"] = commandJson->value("EchoData");

  CurrentResponse.setObject(responseJson);

  // Сериализация и отправка
  transmitSerializedData();
}

void PersoClientConnection::processingFirmwareRequest(
    QJsonObject* commandJson) {
  emit logging("Выполнение команды FirmwareRequest. ");

  //  // Синтаксическая проверка
  //  if (commandJson->value("UCID") == QJsonValue::Undefined) {
  //    emit logging(
  //        "Обнаружена синтаксическая ошибка в команде EchoRequest: отсутствуют
  //        " "эхо-данные. ");
  //    return;
  //  }

  //  QJsonObject responseJson;

  //  // Заголовок ответа на команду
  //  responseJson["CommandName"] = "FirmwareResponse";

  //  // Данные
  //  QFile firmware("firmware.hex");
  //  if (firmware.open(QIODevice::ReadOnly)) {
  //    responseJson["Firmware"] = QString::fromUtf8(firmware.readAll());
  //    firmware.close();
  //  } else {
  //    emit logging("Не найден файл прошивки. ");
  //  }

  //  CurrentResponse.setObject(responseJson);

  //  // Сериализация и отправка
  //  transmitSerializedData();

  QFile firmware("firmware.hex");
  QFileInfo firmwareInfo(firmware);
  if (!firmware.open(QIODevice::ReadOnly)) {
    emit logging("Не найден файл прошивки. ");
    return;
  }

  uint32_t dataBlockSize = firmwareInfo.size();
  QVector<QByteArray*> dataBlock;

  emit logging(QString("Размер файла прошивки: %1.")
                   .arg(QString::number(dataBlockSize)));

  char buffer[10240];
  uint32_t bytesRead = 0;

  for (uint32_t i = 0; i < dataBlockSize; i += bytesRead) {
    bytesRead = bytesRead = firmware.read(buffer, 10240);
    dataBlock.append(new QByteArray(buffer, bytesRead));
    emit logging(
        QString("Размер %1 части блока данных %2.")
            .arg(dataBlock.size())
            .arg(QString::number(dataBlock.at(dataBlock.size() - 1)->size())));
  }

  emit logging(QString("Блок данных состоит из %1 частей.")
                   .arg(QString::number(dataBlock.size())));
}

void PersoClientConnection::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres - " + log);
  else
    emit logging("Unknown - " + log);
}

void PersoClientConnection::on_SocketReadyRead_slot() {
  QByteArray dataBlock;                // Блок данных
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
    deserializator >> dataBlock;
    emit logging("Блок полученных данных: " + dataBlock);

    // Выходим
    break;
  }

  // Осуществляем обработку полученных данных
  processingReceivedDataBlock(&dataBlock);
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
