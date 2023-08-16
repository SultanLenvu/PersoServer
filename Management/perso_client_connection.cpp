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
  connect(Socket,
          QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this,
          &PersoClientConnection::on_SocketError_slot);

  // Таймер для отсчета времени экспирации
  ExpirationTimer = new QTimer(this);
  ExpirationTimer->setInterval(CLIENT_CONNECTION_MAX_DURATION);
  // Если время подключения вышло, то вызываем соответствующий обработчик
  connect(ExpirationTimer, &QTimer::timeout, this,
          &PersoClientConnection::on_ExpirationTimerTimeout_slot);
  // Если время подключения вышло, то останавливаем таймер экспирации
  connect(ExpirationTimer, &QTimer::timeout, ExpirationTimer, &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер экспирации
  connect(Socket,
          QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
          ExpirationTimer, &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::disconnected, ExpirationTimer, &QTimer::stop);

  // Запускаем таймер экспирации
  ExpirationTimer->start();

  // Таймер ожидания для приема блоков данных по частям
  WaitTimer = new QTimer(this);
  WaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  // Если время ожидания вышло, то вызываем соответствующий обработчик
  connect(WaitTimer, &QTimer::timeout, this,
          &PersoClientConnection::on_WaitTimerTimeout_slot);
  // Если время ожидания вышло, то останавливаем таймер ожидания
  connect(WaitTimer, &QTimer::timeout, WaitTimer, &QTimer::stop);
  // Если пришли данные, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::readyRead, WaitTimer, &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::disconnected, WaitTimer, &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер ожидания
  connect(Socket,
          QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
          WaitTimer, &QTimer::stop);
  // Если время подключения вышло, то таймер ожидания останавливается
  connect(ExpirationTimer, &QTimer::timeout, WaitTimer, &QTimer::stop);

  // Данные пока не получены
  ReceivedDataBlockSize = 0;
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

void PersoClientConnection::processingDataBlock(void) {
  QJsonParseError status;
  CurrentCommand = QJsonDocument::fromJson(ReceivedDataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    emit logging("Ошибка парсинга JSON команды. Сброс. ");
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

  // Создаем блок данных для ответа на команду
  createDataBlock();

  // Отправляем сформированный блок данных
  transmitDataBlock();
}

void PersoClientConnection::createDataBlock() {
  emit logging("Формирование блока данных для ответа на команду. ");
  emit logging(
      QString("Размер ответа: %1. Содержание ответа: %2. ")
          .arg(QString::number(
              CurrentResponse.toJson(QJsonDocument::Compact).size()))
          .arg(QString(CurrentResponse.toJson(QJsonDocument::Compact))));

  // Инициализируем блок данных и сериализатор
  TransmittedDataBlock.clear();
  QDataStream serializator(&TransmittedDataBlock, QIODevice::WriteOnly);
  serializator.setVersion(QDataStream::Qt_5_12);

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << CurrentResponse.toJson(QJsonDocument::Compact);
  serializator.device()->seek(0);
  serializator << uint32_t(TransmittedDataBlock.size() - sizeof(uint32_t));
}

void PersoClientConnection::transmitDataBlock() {
  // Если размер блок не превышает максимального размера данных для единоразовой
  // передачи
  if (TransmittedDataBlock.size() < ONETIME_TRANSMIT_DATA_SIZE) {
    // Отправляем блок данных
    Socket->write(TransmittedDataBlock);
    return;
  }

  // В противном случае дробим блок данных на части и последовательно отправляем
  for (int32_t i = 0; i < TransmittedDataBlock.size();
       i += ONETIME_TRANSMIT_DATA_SIZE) {
    Socket->write(TransmittedDataBlock.mid(i, ONETIME_TRANSMIT_DATA_SIZE));
  }
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
}

void PersoClientConnection::processingFirmwareRequest(
    QJsonObject* commandJson) {
  emit logging("Выполнение команды FirmwareRequest. ");

  // Синтаксическая проверка
  if (commandJson->value("UCID") == QJsonValue::Undefined) {
    emit logging(
        "Обнаружена синтаксическая ошибка в команде EchoRequest: отсутствуют "
        "эхо-данные. ");
    return;
  }

  QJsonObject responseJson;

  // Заголовок ответа на команду
  responseJson["CommandName"] = "FirmwareResponse";

  // Данные
  QFile firmware("firmware.hex");
  if (firmware.open(QIODevice::ReadOnly)) {
    responseJson["FirmwareFile"] =
        QString::fromUtf8(firmware.readAll().toBase64());
    firmware.close();
  } else {
    emit logging("Не найден файл прошивки. ");
  }

  CurrentResponse.setObject(responseJson);
}

void PersoClientConnection::proxyLogging(const QString& log) {
  if (sender()->objectName() == "PostgresController")
    emit logging("Postgres - " + log);
  else
    emit logging("Unknown - " + log);
}

void PersoClientConnection::on_SocketReadyRead_slot() {
  QDataStream deserializator(Socket);  // Дессериализатор
  deserializator.setVersion(
      QDataStream::Qt_5_12);  // Настраиваем версию десериализатора

  // Если блок данных еще не начал формироваться
  if (ReceivedDataBlockSize == 0) {
    // Если размер поступивших байт меньше размера поля с размером байт, то
    // блок поступившие данные отбрасываются
    if (Socket->bytesAvailable() < sizeof(uint32_t)) {
      emit logging(
          "Размер полученных данных слишком мал. Ожидается прием следующих "
          "частей. ");
      // Перезапускаем таймер ожидания для следующих частей
      WaitTimer->start();
      return;
    }
    // Сохраняем размер блока данных
    deserializator >> ReceivedDataBlockSize;

    emit logging(QString("Размер принимаемого блока данных: %1.")
                     .arg(QString::number(ReceivedDataBlockSize)));

    // Если размер блока данных слишком большой, то весь блок отбрасывается
    if (ReceivedDataBlockSize > DATA_BLOCK_MAX_SIZE) {
      emit logging("Размер блока данных слишком большой. Сброс. ");
      ReceivedDataBlockSize = 0;
    }
  }

  emit logging(QString("Размер принятых данных: %1. ")
                   .arg(QString::number(Socket->bytesAvailable())));

  // Дожидаемся пока весь блок данных придет целиком
  if (Socket->bytesAvailable() < ReceivedDataBlockSize) {
    emit logging("Блок получен не целиком. Ожидается прием следующих частей. ");
    // Перезапускаем таймер ожидания для следующих частей
    WaitTimer->start();
    return;
  }

  // Если блок был получен целиком, то осуществляем его дессериализацию
  deserializator >> ReceivedDataBlock;
  emit logging("Блок полученных данных: " + ReceivedDataBlock);

  // Осуществляем обработку полученных данных
  processingDataBlock();
}

void PersoClientConnection::on_SocketDisconnected_slot() {
  ExpirationTimer->stop();
  emit logging("Отключился. ");

  //Отправляем сигнал об отключении клиента
  emit disconnected();
}

void PersoClientConnection::on_SocketError_slot(
    QAbstractSocket::SocketError socketError) {
  emit logging(
      QString("Ошибка сети: Код: %1. Описание: %2.")
          .arg(QString::number(socketError).arg(Socket->errorString())));
  Socket->close();
}

void PersoClientConnection::on_ExpirationTimerTimeout_slot() {
  emit logging("Экспирация времени подключения. ");

  // Закрываем соединение
  Socket->close();
}

void PersoClientConnection::on_WaitTimerTimeout_slot() {
  emit logging("Время ожидания вышло. Блок данных сбрасывается. ");
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}
