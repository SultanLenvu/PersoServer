#include "perso_client.h"

PersoClient::PersoClient(uint32_t id, qintptr socketDescriptor) {
  setObjectName("PersoClient");
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

  // Создаем генератор прошивок
  createGenerator();

  // Создаем шаблоны команд
  createCommandTemplates();
}

PersoClient::~PersoClient() {
  sendLog(QString("Клиент %1 удален. ").arg(QString::number(Id)));
}

uint32_t PersoClient::getId() const {
  return Id;
}

void PersoClient::instanceTesting() {
  if (thread() != QCoreApplication::instance()->thread())
    sendLog("Отдельный поток выделен. ");
  else
    sendLog("Отдельный поток не выделен. ");
}

void PersoClient::releaserFinished() {
  ReleaserWaitTimer->stop();
  ReleaserWaiting->quit();

  sendLog("Система выпуска транспондеров завершила выполнение операции. ");
}

void PersoClient::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
  ExtendedLogEnable = settings.value("log_system/extended_enable").toBool();

  MaximumConnectionTime =
      settings.value("perso_client/connection_max_duration").toInt();
}

void PersoClient::sendLog(const QString& log) const {
  if (LogEnable) {
    emit logging("PersoClient - " + log);
  }
}

void PersoClient::createSocket(qintptr socketDescriptor) {
  Socket = new QTcpSocket(this);
  Socket->setSocketDescriptor(socketDescriptor);
  connect(Socket, &QTcpSocket::readyRead, this,
          &PersoClient::on_SocketReadyRead_slot);
  connect(Socket, &QTcpSocket::disconnected, this,
          &PersoClient::on_SocketDisconnected_slot);
  connect(Socket,
          QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this,
          &PersoClient::on_SocketError_slot);
}

void PersoClient::createExpirationTimer() {
  // Таймер для отсчета времени экспирации
  ExpirationTimer = new QTimer(this);
  ExpirationTimer->setInterval(MaximumConnectionTime);
  // Если время подключения вышло, то вызываем соответствующий обработчик
  connect(ExpirationTimer, &QTimer::timeout, this,
          &PersoClient::on_ExpirationTimerTimeout_slot);
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

void PersoClient::createDataBlockWaitTimer() {
  // Таймер ожидания для приема блоков данных по частям
  DataBlockWaitTimer = new QTimer(this);
  DataBlockWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  // Если время ожидания вышло, то вызываем соответствующий обработчик
  connect(DataBlockWaitTimer, &QTimer::timeout, this,
          &PersoClient::on_DataBlockWaitTimerTimeout_slot);
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

void PersoClient::createReleaserWaitTimer() {
  ReleaserWaitTimer = new QTimer(this);
  ReleaserWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  ReleaserWaiting = new QEventLoop(this);

  // Если Releaser зависнет, то вызываем соответствующий обработчик
  connect(ReleaserWaitTimer, &QTimer::timeout, this,
          &PersoClient::on_ReleaserWaitTimerTimeout_slot);
  // Если Releaser зависнет, то выходим из цикла ожидания
  connect(ReleaserWaitTimer, &QTimer::timeout, ReleaserWaiting,
          &QEventLoop::quit);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::disconnected, ReleaserWaitTimer, &QTimer::stop);
}

void PersoClient::createGenerator() {
  Generator = new FirmwareGenerationSystem(this);

  connect(Generator, &FirmwareGenerationSystem::logging, LogSystem::instance(),
          &LogSystem::generate);
}

void PersoClient::createCommandTemplates() {}

void PersoClient::createTransmittedDataBlock() {
  QJsonDocument responseDocument(CurrentResponse);

  sendLog("Формирование блока данных для ответа на команду. ");
  sendLog(QString("Размер ответа: %1.")
              .arg(QString::number(responseDocument.toJson().size())));
  if (ExtendedLogEnable == true) {
    sendLog(QString("Содержание ответа: %1")
                .arg(QString(responseDocument.toJson())));
  }

  // Инициализируем блок данных и сериализатор
  TransmittedDataBlock.clear();
  QDataStream serializator(&TransmittedDataBlock, QIODevice::WriteOnly);
  serializator.setVersion(QDataStream::Qt_5_12);

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << responseDocument.toJson();
  serializator.device()->seek(0);
  serializator << uint32_t(TransmittedDataBlock.size() - sizeof(uint32_t));
}

void PersoClient::transmitDataBlock() {
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

void PersoClient::processReceivedDataBlock(void) {
  QJsonParseError status;
  QJsonDocument requestDocument =
      QJsonDocument::fromJson(ReceivedDataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    sendLog("Ошибка парсинга JSON команды. Сброс. ");
    return;
  } else {
    sendLog("Обработка полученного блока данных. ");
  }

  // Выделяем список пар ключ-значение из JSON-файла
  QJsonObject CommandObject = requestDocument.object();

  // Синтаксическая проверка
  if (CommandObject.value("command_name") == QJsonValue::Undefined) {
    sendLog("Обнаружена синтаксическая ошибка: отсутствует название команды. ");
    return;
  }

  // Вызываем соответствующий обработчик команды
  if (CommandObject.value("command_name").toString() == "Echo") {
    processEcho(&CommandObject);
  } else if (CommandObject.value("command_name").toString() ==
             "Authorization") {
    processAuthorization(&CommandObject);
  } else if (CommandObject.value("command_name").toString() ==
             "TransponderRelease") {
    processTransponderRelease(&CommandObject);
  } else if (CommandObject.value("command_name").toString() ==
             "TransponderReleaseConfirm") {
    processTransponderReleaseConfirm(&CommandObject);
  } else if (CommandObject.value("command_name").toString() ==
             "TransponderRerelease") {
    processTransponderRerelease(&CommandObject);
  } else if (CommandObject.value("command_name").toString() ==
             "TransponderRereleaseConfirm") {
    processTransponderRereleaseConfirm(&CommandObject);
  } else {
    sendLog("Обнаружена синтаксическая ошибка: получена неизвестная команда. ");
    return;
  }
}

void PersoClient::processEcho(QJsonObject* commandJson) {
  sendLog("Выполнение команды Echo. ");

  // Синтаксическая проверка
  if (commandJson->value("data").isUndefined()) {
    sendLog(
        "Обнаружена синтаксическая ошибка в команде Echo: отсутствуют "
        "эхо-данные. ");
    return;
  }

  // Формирование ответа
  CurrentResponse["response_name"] = "Echo";
  CurrentResponse["data"] = commandJson->value("data");
  CurrentResponse["return_status"] = "NoError";
}

void PersoClient::processAuthorization(QJsonObject* commandJson) {
  sendLog("Выполнение команды Authorization. ");
  QMap<QString, QString> authorizationParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["response_name"] = "Authorization";

  // Синтаксическая проверка
  if (commandJson->value("login").isUndefined() ||
      commandJson->value("password").isUndefined()) {
    sendLog("Обнаружена синтаксическая ошибка в команде Authorization. ");
    CurrentResponse["return_status"] = "SyntaxError";
    return;
  }

  // Логика
  authorizationParameters.insert("login",
                                 commandJson->value("login").toString());
  authorizationParameters.insert("password",
                                 commandJson->value("password").toString());
  emit releaserAuthorize_signal(&authorizationParameters, &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  if (ret == TransponderReleaseSystem::Success) {
    CurrentResponse["access"] = "Allowed";
  } else if (ret == TransponderReleaseSystem::ProductionLineNotActive) {
    CurrentResponse["access"] = "NotActive";
  } else if ((ret == TransponderReleaseSystem::Failed) ||
             (ret == TransponderReleaseSystem::ProductionLineMissed)) {
    CurrentResponse["access"] = "NotExist";
  } else {
    CurrentResponse["access"] = "Denied";
  }
  CurrentResponse["return_status"] = "NoError";
}

void PersoClient::processTransponderRelease(QJsonObject* commandJson) {
  sendLog("Выполнение команды TransponderRelease. ");
  QMap<QString, QString> releaseParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["response_name"] = "TransponderRelease";

  // Синтаксическая проверка
  if (commandJson->value("login").isUndefined() ||
      commandJson->value("password").isUndefined()) {
    sendLog("Обнаружена синтаксическая ошибка в команде TransponderRelease.");
    CurrentResponse["return_status"] = "SyntaxError";
    return;
  }

  // Выпуск транспондера
  QMap<QString, QString>* attributes = new QMap<QString, QString>;
  QMap<QString, QString>* masterKeys = new QMap<QString, QString>;
  releaseParameters.insert("login", commandJson->value("login").toString());
  releaseParameters.insert("password",
                           commandJson->value("password").toString());
  emit releaseRelease_signal(&releaseParameters, attributes, masterKeys, &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  if (ret != TransponderReleaseSystem::Success) {
    sendLog("Получена ошибка при выпуске транспондера. ");
    CurrentResponse["return_status"] =
        QString("TransponderReleaseSystemError (%1)").arg(QString::number(ret));
    return;
  }

  sendLog("Генерация прошивки транспондера. ");
  QByteArray firmware;
  Generator->generate(attributes, masterKeys, &firmware);
  CurrentResponse["firmware"] = QString::fromUtf8(firmware.toBase64());
  CurrentResponse["return_status"] = "NoError";
}

void PersoClient::processTransponderReleaseConfirm(QJsonObject* commandJson) {
  sendLog("Выполнение команды TransponderReleaseConfirm. ");
  QMap<QString, QString> confirmParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["response_name"] = "TransponderReleaseConfirm";

  // Синтаксическая проверка
  if (commandJson->value("login").isUndefined() ||
      commandJson->value("password").isUndefined() ||
      commandJson->value("ucid").isUndefined()) {
    sendLog(
        "Обнаружена синтаксическая ошибка в команде "
        "TransponderReleaseConfirm.");
    CurrentResponse["return_status"] = "SyntaxError";
    return;
  }

  // Подтверждение выпуска транспондера
  QMap<QString, QString>* transponderData = new QMap<QString, QString>;
  confirmParameters.insert("login", commandJson->value("login").toString());
  confirmParameters.insert("password",
                           commandJson->value("password").toString());
  confirmParameters.insert("ucid", commandJson->value("ucid").toString());
  emit releaserConfirmRelease_signal(&confirmParameters, transponderData, &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  if (ret != TransponderReleaseSystem::Success) {
    sendLog("Получена ошибка при подтверждении выпуска транспондера. ");
    CurrentResponse["return_status"] =
        QString("TransponderReleaseSystemError (%1)").arg(QString::number(ret));
    return;
  }

  CurrentResponse["sn"] = transponderData->value("sn");
  CurrentResponse["pan"] = transponderData->value("pan");
  CurrentResponse["box_id"] = transponderData->value("box_id");
  CurrentResponse["pallet_id"] = transponderData->value("pallet_id");
  CurrentResponse["order_id"] = transponderData->value("order_id");
  CurrentResponse["issuer_name"] = transponderData->value("issuer_name");
  CurrentResponse["transponder_model"] =
      transponderData->value("transponder_model");
  CurrentResponse["return_status"] = "NoError";
}

void PersoClient::processTransponderRerelease(QJsonObject* commandJson) {
  sendLog("Выполнение команды TransponderRerelease. ");
  QMap<QString, QString> rereleaseParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["response_name"] = "TransponderRerelease";

  // Синтаксическая проверка
  if (commandJson->value("login").isUndefined() ||
      commandJson->value("password").isUndefined() ||
      commandJson->value("pan").isUndefined()) {
    sendLog("Обнаружена синтаксическая ошибка в команде TransponderRerelease.");
    CurrentResponse["return_status"] = "SyntaxError";
    return;
  }

  // Перевыпуск транспондера
  QMap<QString, QString>* attributes = new QMap<QString, QString>;
  QMap<QString, QString>* masterKeys = new QMap<QString, QString>;
  rereleaseParameters.insert("login", commandJson->value("login").toString());
  rereleaseParameters.insert("password",
                             commandJson->value("password").toString());
  rereleaseParameters.insert("personal_account_number",
                             commandJson->value("pan").toString().leftJustified(
                                 FULL_PAN_CHAR_LENGTH, QChar('F')));
  emit releaserRerelease_signal(&rereleaseParameters, attributes, masterKeys,
                                &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  if (ret != TransponderReleaseSystem::Success) {
    sendLog("Получена ошибка при перевыпуске транспондера. ");
    CurrentResponse["return_status"] =
        QString("TransponderReleaseSystemError (%1)").arg(QString::number(ret));
    return;
  }

  sendLog("Генерация прошивки транспондера. ");
  QByteArray firmware;
  Generator->generate(attributes, masterKeys, &firmware);
  CurrentResponse["firmware"] = QString::fromUtf8(firmware.toBase64());
  CurrentResponse["return_status"] = "NoError";
}

void PersoClient::processTransponderRereleaseConfirm(QJsonObject* commandJson) {
  sendLog("Выполнение команды TransponderRereleaseConfirm. ");
  QMap<QString, QString> confirmParameters;
  TransponderReleaseSystem::ReturnStatus ret =
      TransponderReleaseSystem::Undefined;

  // Заголовок ответа
  CurrentResponse["response_name"] = "TransponderRereleaseConfirm";

  // Синтаксическая проверка
  if (commandJson->value("login").isUndefined() ||
      commandJson->value("password").isUndefined() ||
      commandJson->value("pan").isUndefined() ||
      commandJson->value("ucid").isUndefined()) {
    sendLog(
        "Обнаружена синтаксическая ошибка в команде "
        "TransponderRereleaseConfirm.");
    CurrentResponse["return_status"] = "SyntaxError";
    return;
  }

  // Подтверждение перевыпуска транспондера
  QMap<QString, QString>* transponderData = new QMap<QString, QString>;
  confirmParameters.insert("login", commandJson->value("login").toString());
  confirmParameters.insert("password",
                           commandJson->value("password").toString());
  confirmParameters.insert("personal_account_number",
                           commandJson->value("pan").toString().leftJustified(
                               FULL_PAN_CHAR_LENGTH, QChar('F')));
  confirmParameters.insert("ucid", commandJson->value("ucid").toString());
  emit releaserConfirmRerelease_signal(&confirmParameters, transponderData,
                                       &ret);

  // Ожидаем завершения работы
  while (ret == TransponderReleaseSystem::Undefined) {
    QCoreApplication::processEvents();
  }

  if (ret != TransponderReleaseSystem::Success) {
    sendLog("Получена ошибка при подтверждении перевыпуска транспондера. ");
    CurrentResponse["return_status"] =
        QString("TransponderReleaseSystemError (%1)").arg(QString::number(ret));
    return;
  }

  CurrentResponse["sn"] = transponderData->value("sn");
  CurrentResponse["pan"] = transponderData->value("pan");
  CurrentResponse["box_id"] = transponderData->value("box_id");
  CurrentResponse["pallet_id"] = transponderData->value("pallet_id");
  CurrentResponse["order_id"] = transponderData->value("order_id");
  CurrentResponse["issuer_name"] = transponderData->value("issuer_name");
  CurrentResponse["transponder_model"] =
      transponderData->value("transponder_model");
  CurrentResponse["return_status"] = "NoError";
}

void PersoClient::on_SocketReadyRead_slot() {
  QDataStream deserializator(Socket);  // Дессериализатор
  deserializator.setVersion(
      QDataStream::Qt_5_12);  // Настраиваем версию десериализатора

  // Если блок данных еще не начал формироваться
  if (ReceivedDataBlockSize == 0) {
    // Если размер поступивших байт меньше размера поля с размером байт, то
    // блок поступившие данные отбрасываются
    if (Socket->bytesAvailable() < static_cast<int64_t>(sizeof(uint32_t))) {
      sendLog(
          "Размер полученных данных слишком мал. Ожидается прием следующих "
          "частей. ");
      // Перезапускаем таймер ожидания для следующих частей
      DataBlockWaitTimer->start();
      return;
    }
    // Сохраняем размер блока данных
    deserializator >> ReceivedDataBlockSize;

    sendLog(QString("Размер принимаемого блока данных: %1.")
                .arg(QString::number(ReceivedDataBlockSize)));

    // Если размер блока данных слишком большой, то весь блок отбрасывается
    if (ReceivedDataBlockSize > DATA_BLOCK_MAX_SIZE) {
      sendLog("Размер блока данных слишком большой. Сброс. ");
      ReceivedDataBlockSize = 0;
    }
  }

  sendLog(QString("Размер принятых данных: %1. ")
              .arg(QString::number(Socket->bytesAvailable())));

  // Дожидаемся пока весь блок данных придет целиком
  if (Socket->bytesAvailable() < ReceivedDataBlockSize) {
    sendLog("Блок получен не целиком. Ожидается прием следующих частей. ");
    // Перезапускаем таймер ожидания для следующих частей
    DataBlockWaitTimer->start();
    return;
  }

  // Если блок был получен целиком, то осуществляем его дессериализацию
  deserializator >> ReceivedDataBlock;
  if (ExtendedLogEnable == true) {
    sendLog("Блок полученных данных: " + ReceivedDataBlock);
  }

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

void PersoClient::on_SocketDisconnected_slot() {
  sendLog("Отключился. ");

  //Отправляем сигнал об отключении клиента
  emit disconnected();
}

void PersoClient::on_SocketError_slot(
    QAbstractSocket::SocketError socketError) {
  // Если клиент отключился не самостоятельно
  if (socketError != 1) {
    sendLog(QString("Ошибка сети: Код: %1. Описание: %2.")
                .arg(QString::number(socketError))
                .arg(Socket->errorString()));
    Socket->close();
  }
}

void PersoClient::on_ExpirationTimerTimeout_slot() {
  sendLog("Экспирация времени подключения. ");

  // Закрываем соединение
  Socket->close();
}

void PersoClient::on_DataBlockWaitTimerTimeout_slot() {
  sendLog("Время ожидания вышло. Блок данных сбрасывается. ");
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}

void PersoClient::on_ReleaserWaitTimerTimeout_slot() {
  sendLog("Система выпуска транспондеров зависла. ");
  // Закрываем соединение
  Socket->close();
}
