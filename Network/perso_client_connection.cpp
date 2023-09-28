#include "perso_client_connection.h"

PersoClientConnection::PersoClientConnection(uint32_t id,
                                             qintptr socketDescriptor) {
  setObjectName("PersoClientConnection");
  Id = id;

  // Загружаем настройки
  loadSettings();

  // Создаем сокет в соответствии с системным дескриптором
  createSocket(socketDescriptor);

  // Блок данных пока не получен
  ReceivedDataBlockSize = 0;

  // Создаем таймер экспирации подключения
  createExpirationTimer();

  // Создаем таймер для приема блоков данных частями
  createDataBlockWaitTimer();
}

PersoClientConnection::~PersoClientConnection() {}

uint32_t PersoClientConnection::getId() {
  return Id;
}

void PersoClientConnection::applySettings() {
  QSettings settings(ORGANIZATION_NAME, PROGRAM_NAME);
}

void PersoClientConnection::instanceTesting() {
  if (thread() != QApplication::instance()->thread())
    emit logging("Отдельный поток выделен. ");
  else
    emit logging("Отдельный поток не выделен. ");
}

void PersoClientConnection::releaserFinished() {
  ReleaserWaitTimer->stop();
  ReleaserWaiting->quit();

  emit logging("Система выпуска транспондеров завершила выполнение операции. ");
}

void PersoClientConnection::loadSettings() {
  QSettings settings;
  MaximumConnectionTime =
      settings.value("PersoHost/ClientConnection/MaxDuration").toInt();
}

void PersoClientConnection::createSocket(qintptr socketDescriptor) {
  Socket = new QTcpSocket(this);
  Socket->setSocketDescriptor(socketDescriptor);
  connect(Socket, &QTcpSocket::readyRead, this,
          &PersoClientConnection::on_SocketReadyRead_slot);
  connect(Socket, &QTcpSocket::disconnected, this,
          &PersoClientConnection::on_SocketDisconnected_slot);
  connect(Socket,
          QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this,
          &PersoClientConnection::on_SocketError_slot);
}

void PersoClientConnection::createExpirationTimer() {
  // Таймер для отсчета времени экспирации
  ExpirationTimer = new QTimer(this);
  ExpirationTimer->setInterval(MaximumConnectionTime);
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
}

void PersoClientConnection::createDataBlockWaitTimer() {
  // Таймер ожидания для приема блоков данных по частям
  DataBlockWaitTimer = new QTimer(this);
  DataBlockWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  // Если время ожидания вышло, то вызываем соответствующий обработчик
  connect(DataBlockWaitTimer, &QTimer::timeout, this,
          &PersoClientConnection::on_DataBlockWaitTimerTimeout_slot);
  // Если время ожидания вышло, то останавливаем таймер ожидания
  connect(DataBlockWaitTimer, &QTimer::timeout, DataBlockWaitTimer,
          &QTimer::stop);
  // Если пришли данные, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::readyRead, DataBlockWaitTimer, &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::disconnected, DataBlockWaitTimer, &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер ожидания
  connect(Socket,
          QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
          DataBlockWaitTimer, &QTimer::stop);
  // Если время подключения вышло, то таймер ожидания останавливается
  connect(ExpirationTimer, &QTimer::timeout, DataBlockWaitTimer, &QTimer::stop);
}

void PersoClientConnection::createReleaserWaitTimer() {
  ReleaserWaitTimer = new QTimer(this);
  ReleaserWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  ReleaserWaiting = new QEventLoop(this);

  // Если Releaser зависнет, то вызываем соответствующий обработчик
  connect(ReleaserWaitTimer, &QTimer::timeout, this,
          &PersoClientConnection::on_ReleaserWaitTimerTimeout_slot);
  // Если Releaser зависнет, то выходим из цикла ожидания
  connect(ReleaserWaitTimer, &QTimer::timeout, ReleaserWaiting,
          &QEventLoop::quit);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::disconnected, ReleaserWaitTimer, &QTimer::stop);
}

void PersoClientConnection::createTransmittedDataBlock() {
  QJsonDocument responseDocument(CurrentResponse);

  emit logging("Формирование блока данных для ответа на команду. ");
  emit logging(QString("Размер ответа: %1. Содержание ответа: %2. ")
                   .arg(QString::number(responseDocument.toJson().size()))
                   .arg(QString(responseDocument.toJson())));

  // Инициализируем блок данных и сериализатор
  TransmittedDataBlock.clear();
  QDataStream serializator(&TransmittedDataBlock, QIODevice::WriteOnly);
  serializator.setVersion(QDataStream::Qt_5_12);

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << responseDocument.toJson();
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

void PersoClientConnection::processReceivedDataBlock(void) {
  QJsonParseError status;
  QJsonDocument requestDocument =
      QJsonDocument::fromJson(ReceivedDataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    emit logging("Ошибка парсинга JSON команды. Сброс. ");
    return;
  } else {
    emit logging("Обработка полученного блока данных. ");
  }

  // Выделяем список пар ключ-значение из JSON-файла
  QJsonObject CommandObject = requestDocument.object();

  // Синтаксическая проверка
  if (CommandObject.value("CommandName") == QJsonValue::Undefined) {
    emit logging(
        "Обнаружена синтаксическая ошибка: отсутствует название команды. ");
    return;
  }

  // Вызываем соответствующий обработчик команды
  if (CommandObject.value("CommandName").toString() == "Echo") {
    processEcho(&CommandObject);
  } else if (CommandObject.value("CommandName").toString() == "Authorization") {
    processAuthorization(&CommandObject);
  } else if (CommandObject.value("CommandName").toString() ==
             "TransponderRelease") {
    processTransponderRelease(&CommandObject);
  } else if (CommandObject.value("CommandName").toString() ==
             "TransponderReleaseConfirm") {
    processTransponderReleaseConfirm(&CommandObject);
  } else if (CommandObject.value("CommandName").toString() ==
             "TransponderRerelease") {
    processTransponderRerelease(&CommandObject);
  } else if (CommandObject.value("CommandName").toString() ==
             "TransponderRereleaseConfirm") {
    processTransponderRereleaseConfirm(&CommandObject);
  } else {
    emit logging(
        "Обнаружена синтаксическая ошибка: получена неизвестная команда. ");
    return;
  }
}

void PersoClientConnection::processEcho(QJsonObject* commandJson) {
  emit logging("Выполнение команды Echo. ");

  // Синтаксическая проверка
  if (commandJson->value("Data").isUndefined()) {
    emit logging(
        "Обнаружена синтаксическая ошибка в команде Echo: отсутствуют "
        "эхо-данные. ");
    return;
  }

  // Формирование ответа
  CurrentResponse["ResponseName"] = "Echo";
  CurrentResponse["Data"] = commandJson->value("Data");
}

void PersoClientConnection::processAuthorization(QJsonObject* commandJson) {
  emit logging("Выполнение команды Authorization. ");
  QMap<QString, QString> authorizationParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["ResponseName"] = "Authorization";

  // Синтаксическая проверка
  if (commandJson->value("Login").isUndefined() ||
      commandJson->value("Login").toString().isEmpty() ||
      commandJson->value("Password").isUndefined() ||
      commandJson->value("Password").toString().isEmpty()) {
    emit logging("Обнаружена синтаксическая ошибка в команде Authorization. ");
    CurrentResponse["Access"] = "Denied";
    return;
  }

  // Логика
  authorizationParameters.insert("Login",
                                 commandJson->value("Login").toString());
  authorizationParameters.insert("Password",
                                 commandJson->value("Password").toString());
  emit authorize_signal(&authorizationParameters, &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  if (ret == TransponderReleaseSystem::Success) {
    CurrentResponse["Access"] = "Allowed";
  } else {
    CurrentResponse["Access"] = "Denied";
  }
}

void PersoClientConnection::processTransponderRelease(
    QJsonObject* commandJson) {
  emit logging("Выполнение команды TransponderRelease. ");
  QMap<QString, QString> authorizationParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["ResponseName"] = "Authorization";

  // Синтаксическая проверка
  if (commandJson->value("UCID") == QJsonValue::Undefined) {
    emit logging(
        "Обнаружена синтаксическая ошибка в команде TransponderRelease: "
        "отсутствуют "
        "эхо-данные. ");
    return;
  }

  // Логика
  authorizationParameters.insert("Login",
                                 commandJson->value("Login").toString());
  authorizationParameters.insert("Password",
                                 commandJson->value("Password").toString());
  emit authorize_signal(&authorizationParameters, &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  QFile firmware("firmware.hex");
  if (firmware.open(QIODevice::ReadOnly)) {
    CurrentResponse["FirmwareFile"] = QString::fromUtf8(firmware.readAll());
    firmware.close();
  } else {
    emit logging("Не найден файл прошивки. ");
  }
}

void PersoClientConnection::processTransponderReleaseConfirm(
    QJsonObject* commandJson) {}

void PersoClientConnection::processTransponderRerelease(
    QJsonObject* commandJson) {}

void PersoClientConnection::processTransponderRereleaseConfirm(
    QJsonObject* commandJson) {}

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
    if (Socket->bytesAvailable() < static_cast<int64_t>(sizeof(uint32_t))) {
      emit logging(
          "Размер полученных данных слишком мал. Ожидается прием следующих "
          "частей. ");
      // Перезапускаем таймер ожидания для следующих частей
      DataBlockWaitTimer->start();
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
    DataBlockWaitTimer->start();
    return;
  }

  // Если блок был получен целиком, то осуществляем его дессериализацию
  deserializator >> ReceivedDataBlock;
  emit logging("Блок полученных данных: " + ReceivedDataBlock);

  // Осуществляем обработку полученных данных
  processReceivedDataBlock();

  // Создаем блок данных для ответа на команду
  createTransmittedDataBlock();

  // Отправляем сформированный блок данных
  transmitDataBlock();

  // Очистка
  CurrentCommand = QJsonObject();
  CurrentResponse = QJsonObject();
}

void PersoClientConnection::on_SocketDisconnected_slot() {
  emit logging("Отключился. ");

  //Отправляем сигнал об отключении клиента
  emit disconnected();
}

void PersoClientConnection::on_SocketError_slot(
    QAbstractSocket::SocketError socketError) {
  // Если клиент отключился не самостоятельно
  if (socketError != 1) {
    emit logging(QString("Ошибка сети: Код: %1. Описание: %2.")
                     .arg(QString::number(socketError))
                     .arg(Socket->errorString()));
    Socket->close();
  }
}

void PersoClientConnection::on_ExpirationTimerTimeout_slot() {
  emit logging("Экспирация времени подключения. ");

  // Закрываем соединение
  Socket->close();
}

void PersoClientConnection::on_DataBlockWaitTimerTimeout_slot() {
  emit logging("Время ожидания вышло. Блок данных сбрасывается. ");
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}

void PersoClientConnection::on_ReleaserWaitTimerTimeout_slot() {
  emit logging("Система выпуска транспондеров зависла. ");
  // Закрываем соединение
  Socket->close();
}
