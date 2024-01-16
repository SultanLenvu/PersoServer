#include "perso_client.h"

PersoClient::PersoClient(QObject* parent) : QObject(parent) {
  setObjectName("PersoClient");
  loadSettings();

  // Создаем сокет
  createSocket();

  // Создаем таймеры
  createTimers();

  // Готовы к выполнению команд
  CurrentState = Ready;

  createResponseHandlers();
  createResponseTemplates();
  createServerStatusMatchTable();
}

PersoClient::~PersoClient() {
  if (Socket->isOpen())
    Socket->disconnectFromHost();
}

PersoClient::ReturnStatus PersoClient::connectToServer() {
  // Подключаемся к серверу персонализации
  if (!processingServerConnection()) {
    return ServerConnectionError;
  }

  return Completed;
}

PersoClient::ReturnStatus PersoClient::disconnectFromServer() {
  if (Socket->isOpen()) {
    Socket->disconnectFromHost();
    sendLog("Отключение от сервера персонализации. ");
  } else {
    sendLog("Подключение не было установлено. ");
  }

  return Completed;
}

PersoClient::ReturnStatus PersoClient::requestEcho() {
  // Создаем запрос
  CurrentState = CreatingRequest;
  createEcho();

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestAuthorize(
    const QHash<QString, QString>* requestData) {
  // Создаем запрос
  CurrentState = CreatingRequest;

  // Проверка на существование
  if (!requestData) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }
  createAuthorization(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestTransponderRelease(
    const QHash<QString, QString>* requestData,
    QFile* firmware,
    QHash<QString, QString>* responseData) {
  // Проверка на существование
  if ((!requestData) || (!firmware) || (!responseData)) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }
  Firmware = firmware;
  ResponseData = responseData;

  // Создаем запрос
  CurrentState = CreatingRequest;
  createTransponderRelease(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestTransponderReleaseConfirm(
    const QHash<QString, QString>* requestData) {
  // Проверка на существование
  if (!requestData) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }

  // Создаем запрос
  CurrentState = CreatingRequest;
  createTransponderReleaseConfirm(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestTransponderRerelease(
    const QHash<QString, QString>* requestData,
    QFile* firmware,
    QHash<QString, QString>* responseData) {
  // Проверка на существование
  if ((!requestData) || (!firmware) || (!responseData)) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }
  Firmware = firmware;
  ResponseData = responseData;

  // Создаем запрос
  CurrentState = CreatingRequest;
  createTransponderRerelease(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestTransponderRereleaseConfirm(
    const QHash<QString, QString>* requestData) {
  // Проверка на существование
  if (!requestData) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }

  // Создаем запрос
  CurrentState = CreatingRequest;
  createTransponderRereleaseConfirm(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestProductionLineRollback(
    const QHash<QString, QString>* requestData) {
  // Проверка на существование
  if (!requestData) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }

  // Создаем запрос
  CurrentState = CreatingRequest;
  createProductionLineRollback(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestBoxStickerPrint(
    const QHash<QString, QString>* requestData) {
  // Проверка на существование
  if (!requestData) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }

  // Создаем запрос
  CurrentState = CreatingRequest;
  createBoxStickerPrint(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestBoxStickerReprint() {
  // Создаем запрос
  CurrentState = CreatingRequest;
  createBoxStickerReprint();

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestPalletStickerPrint(
    const QHash<QString, QString>* requestData) {
  // Проверка на существование
  if (!requestData) {
    sendLog("Получены не корректные параметры запроса. Сброс.");
    return RequestParameterError;
  }

  // Создаем запрос
  CurrentState = CreatingRequest;
  createPalletStickerPrint(requestData);

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

PersoClient::ReturnStatus PersoClient::requestPalletStickerReprint() {
  // Создаем запрос
  CurrentState = CreatingRequest;
  createPalletStickerReprint();

  // Отправляем сформированный блок данных
  return transmitDataBlock();
}

void PersoClient::applySettings() {
  sendLog("Применение новых настроек. ");
  loadSettings();
}

/*
 * Приватные  методы
 */

void PersoClient::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
  ExtendedLoggingEnable = settings.value("log_system/extended_enable").toBool();

  PersoServerAddress = QHostAddress(
      settings.value("perso_client/server_ip").toString());
  PersoServerPort = settings.value("perso_client/server_port").toInt();
}

void PersoClient::sendLog(const QString& log) {
  if (LogEnable) {
    emit logging(QString("%1 - %2").arg(objectName(), log));
  }
}

bool PersoClient::processingServerConnection() {
  sendLog("Подключение к серверу персонализации. ");
  // Подключаемся к серверу персонализации
  Socket->connectToHost(PersoServerAddress, PersoServerPort);

  // Ожидаем подключения или отказа
  sendLog("Ожидание ответа от сервера. ");
  WaitTimer->start();
  WaitingLoop->exec();

  // Если соединение не удалось установить
  if (!Socket->isOpen()) {
    sendLog(
        "Не удалось установить соединение с сервером персонализации. Сброс. ");
    return false;
  }

  return true;
}

bool PersoClient::processingDataBlock() {
  QJsonParseError status;
  QJsonDocument responseDocument =
      QJsonDocument::fromJson(ReceivedDataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    sendLog("Ошибка парсинга JSON команды. ");
    return false;
  }

  sendLog(QString("Размер полученного блока данных: %1.")
              .arg(QString::number(responseDocument.toJson().size())));
  if (ExtendedLoggingEnable == true) {
    sendLog(QString("Содержание полученного блока данных: %1 ")
                .arg(QString(responseDocument.toJson())));
  }

  // Выделяем список пар ключ-значение из JSON-файла
  CurrentResponse = responseDocument.object();
  return true;
}

void PersoClient::createTransmittedDataBlock(void) {
  QJsonDocument requestDocument(CurrentCommand);

  sendLog(QString("Размер команды: %1 ")
              .arg(QString::number(requestDocument.toJson().size())));
  if (ExtendedLoggingEnable == true) {
    sendLog(QString("Содержание команды: %1 ")
                .arg(QString(requestDocument.toJson())));
  }
  sendLog("Формирование блока данных для команды. ");

  // Инициализируем блок данных и сериализатор
  TransmittedDataBlock.clear();
  QDataStream serializator(&TransmittedDataBlock, QIODevice::WriteOnly);
  serializator.setVersion(QDataStream::Qt_5_12);

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << requestDocument.toJson();
  serializator.device()->seek(0);
  serializator << uint32_t(TransmittedDataBlock.size() - sizeof(uint32_t));
}

PersoClient::ReturnStatus PersoClient::transmitDataBlock() {
  ReturnStatus ret;
  // Ответный блок данных еще не получен
  ReceivedDataBlockSize = 0;

  // Создаем блок данных для команды
  createTransmittedDataBlock();

  // Подключаемся к серверу персонализации
  CurrentState = WaitingServerConnection;
  if (!processingServerConnection()) {
    return ServerConnectionError;
  }

  // Если размер блок не превышает максимального размера данных для единоразовой
  // передачи
  if (TransmittedDataBlock.size() < ONETIME_TRANSMIT_DATA_SIZE) {
    // Отправляем блок данных
    Socket->write(TransmittedDataBlock);
  } else {
    // В противном случае дробим блок данных на части и последовательно
    // отправляем
    for (int32_t i = 0; i < TransmittedDataBlock.size();
         i += ONETIME_TRANSMIT_DATA_SIZE) {
      Socket->write(TransmittedDataBlock.mid(i, ONETIME_TRANSMIT_DATA_SIZE));
    }
  }

  // Ожидаем ответ
  CurrentState = WaitingResponse;
  WaitTimer->start();
  WaitingLoop->exec();

  // Отключаемся от сервера
  Socket->close();

  // Если сервер ничего не ответил
  if (ReceivedDataBlock.isEmpty()) {
    return ServerNotResponding;
  }

  // Осуществляем обработку полученных данных
  CurrentState = ProcessingResponse;
  if (!processingDataBlock()) {
    return ResponseParsingError;
  }

  // Проверка имени команды
  if (CurrentCommand.value("command_name") !=
      CurrentResponse.value("response_name")) {
    sendLog(QString("Получен некорректный заголовок ответа: %1. ")
                .arg(CurrentResponse.value("response_name").toString()));
    return ResponseSyntaxError;
  }

  // Проверка наличия статуса
  if (CurrentResponse.value("return_status").isUndefined()) {
    sendLog(QString("В ответа на команду %1 отсутствует статус возврата. ")
                .arg(CurrentResponse.value("response_name").toString()));
    return ResponseSyntaxError;
  }

  // Проверка статуса
  if (CurrentResponse.value("return_status").toString() != "0") {
    return ServerStatusMatchTable.value(
        CurrentResponse.value("return_status").toString());
  }

  // Синтаксическая проверка ответа
  QVector<QString>* currentTemplate =
      ResponseTemplates.value(CurrentResponse.value("response_name").toString())
          .get();
  if (!currentTemplate) {
    sendLog(QString("Не удалось найти соответствующий обработчик ответа: %1. ")
                .arg(CurrentResponse.value("response_name").toString()));
    return ResponseSyntaxError;
  }

  QVector<QString>::iterator it;
  for (it = currentTemplate->begin(); it != currentTemplate->end(); it++) {
    if (!CurrentResponse.contains(*it)) {
      return ResponseSyntaxError;
    }
  }

  ret = (this->*ResponseHandlers.value(
                    CurrentCommand.value("command_name").toString()))();

  // Замыкаем машину состояний
  CurrentState = Ready;

  // Очищаем команду и ответ на нее
  CurrentCommand = QJsonObject();
  CurrentResponse = QJsonObject();

  return ret;
}

void PersoClient::createEcho(void) {
  sendLog("Формирование команды echo. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "echo";

  // Тело команды
  CurrentCommand["data"] = "test";
}

void PersoClient::createAuthorization(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды athorization. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "authorization";

  // Тело команды
  CurrentCommand["login"] = requestData->value("login");
  CurrentCommand["password"] = requestData->value("password");
}

void PersoClient::createTransponderRelease(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды transponder_release. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "transponder_release";

  // Тело команды
  CurrentCommand["login"] = requestData->value("login");
  CurrentCommand["password"] = requestData->value("password");
}

void PersoClient::createTransponderReleaseConfirm(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды transponder_release_confirm. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "transponder_release_confirm";

  // Тело команды
  CurrentCommand["login"] = requestData->value("login");
  CurrentCommand["password"] = requestData->value("password");
  CurrentCommand["ucid"] = requestData->value("ucid");
}

void PersoClient::createTransponderRerelease(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды transponder_rerelease. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "transponder_rerelease";

  // Тело команды
  CurrentCommand["login"] = requestData->value("login");
  CurrentCommand["password"] = requestData->value("password");
  CurrentCommand["pan"] = requestData->value("pan");
}

void PersoClient::createTransponderRereleaseConfirm(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды transponder_rerelease_confirm. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "transponder_rerelease_confirm";

  // Тело команды
  CurrentCommand["login"] = requestData->value("login");
  CurrentCommand["password"] = requestData->value("password");
  CurrentCommand["pan"] = requestData->value("pan");
  CurrentCommand["ucid"] = requestData->value("ucid");
}

void PersoClient::createProductionLineRollback(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды production_line_rollback. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "production_line_rollback";

  // Тело команды
  CurrentCommand["login"] = requestData->value("login");
  CurrentCommand["password"] = requestData->value("password");
}

void PersoClient::createBoxStickerPrint(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды print_box_sticker. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "print_box_sticker";

  // Тело команды
  CurrentCommand["pan"] = requestData->value("pan");
}

void PersoClient::createBoxStickerReprint() {
  sendLog("Формирование команды print_last_box_sticker. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "print_last_box_sticker";
}

void PersoClient::createPalletStickerPrint(
    const QHash<QString, QString>* requestData) {
  sendLog("Формирование команды print_pallet_sticker. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "print_pallet_sticker";

  // Тело команды
  CurrentCommand["pan"] = requestData->value("pan");

  // Создаем блок данных для команды
  createTransmittedDataBlock();
}

void PersoClient::createPalletStickerReprint() {
  sendLog("Формирование команды print_last_pallet_sticker. ");

  // Заголовок команды
  CurrentCommand["command_name"] = "print_last_pallet_sticker";
}

PersoClient::ReturnStatus PersoClient::processEcho(void) {
  sendLog("Обработка ответа на команду Echo. ");

  sendLog("Команда Echo успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processAuthorization(void) {
  sendLog("Обработка ответа на команду TransponderRelease. ");

  sendLog("Команда Authorization успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processTransponderRelease() {
  sendLog("Обработка ответа на команду transponder_release. ");

  // Сохраняем присланный файл прошивки
  if (!Firmware->open(QIODevice::WriteOnly)) {
    sendLog("Не удалось сохранить файл прошивки. ");
    return FirmwareFileSavingError;
  }

  // Сохраняем прошивку в файл
  Firmware->write(QByteArray::fromBase64(
      CurrentResponse.value("firmware").toString().toUtf8()));
  Firmware->close();

  // Данные транспондера
  ResponseData->insert("sn", CurrentResponse.value("sn").toString());
  ResponseData->insert("pan", CurrentResponse.value("pan").toString());
  ResponseData->insert("box_id", CurrentResponse.value("box_id").toString());
  ResponseData->insert("pallet_id",
                       CurrentResponse.value("pallet_id").toString());
  ResponseData->insert("order_id",
                       CurrentResponse.value("order_id").toString());
  ResponseData->insert("issuer_name",
                       CurrentResponse.value("issuer_name").toString());
  ResponseData->insert("transponder_model",
                       CurrentResponse.value("transponder_model").toString());

  sendLog("Команда TransponderRelease успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processTransponderReleaseConfirm() {
  sendLog("Обработка ответа на команду transponder_release_confirm. ");

  return Completed;
  sendLog("Команда TransponderReleaseConfirm успешно выполнена. ");
}

PersoClient::ReturnStatus PersoClient::processTransponderRerelease() {
  sendLog("Обработка ответа на команду transponder_rerelease. ");

  // Сохраняем присланный файл прошивки
  if (!Firmware->open(QIODevice::WriteOnly)) {
    sendLog("Не удалось сохранить файл прошивки. ");
    return ResponseSyntaxError;
  }

  // Сохраняем прошивку в файл
  Firmware->write(QByteArray::fromBase64(
      CurrentResponse.value("firmware").toString().toUtf8()));
  Firmware->close();

  // Данные транспондера
  ResponseData->insert("sn", CurrentResponse.value("sn").toString());
  ResponseData->insert("pan", CurrentResponse.value("pan").toString());
  ResponseData->insert("box_id", CurrentResponse.value("box_id").toString());
  ResponseData->insert("pallet_id",
                       CurrentResponse.value("pallet_id").toString());
  ResponseData->insert("order_id",
                       CurrentResponse.value("order_id").toString());
  ResponseData->insert("issuer_name",
                       CurrentResponse.value("issuer_name").toString());
  ResponseData->insert("transponder_model",
                       CurrentResponse.value("transponder_model").toString());

  sendLog("Команда transponder_rerelease успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processTransponderRereleaseConfirm() {
  sendLog("Обработка ответа на команду transponder_rerelease_confirm. ");

  sendLog("Команда transponder_rerelease_confirm успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processProductionLineRollback() {
  sendLog("Обработка ответа на команду production_line_rollback. ");

  sendLog("Команда production_line_rollback успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processPrintBoxSticker() {
  sendLog("Обработка ответа на команду print_box_sticker. ");

  sendLog("Команда print_box_sticker успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processPrintLastBoxSticker() {
  sendLog("Обработка ответа на команду print_last_box_sticker. ");

  sendLog("Команда print_last_box_sticker успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processPrintPalletSticker() {
  sendLog("Обработка ответа на команду print_pallet_sticker. ");

  sendLog("Команда print_pallet_sticker успешно выполнена. ");
  return Completed;
}

PersoClient::ReturnStatus PersoClient::processPrintLastPalletSticker() {
  sendLog("Обработка ответа на команду print_last_pallet_sticker. ");

  sendLog("Команда print_last_pallet_sticker успешно выполнена. ");
  return Completed;
}

void PersoClient::createTimers() {
  WaitTimer = new QTimer(this);
  WaitTimer->setInterval(PERSO_SERVER_CONNECTION_WAITING_TIME);
  connect(WaitTimer, &QTimer::timeout, this,
          &PersoClient::on_WaitTimerTimeout_slot);
  connect(WaitTimer, &QTimer::timeout, WaitTimer, &QTimer::stop);
  connect(Socket, &QTcpSocket::readyRead, WaitTimer, &QTimer::stop);
  connect(Socket, &QTcpSocket::connected, WaitTimer, &QTimer::stop);
  connect(Socket, &QTcpSocket::disconnected, WaitTimer, &QTimer::stop);
  connect(Socket, &QTcpSocket::errorOccurred, WaitTimer, &QTimer::stop);

  WaitingLoop = new QEventLoop(this);
  connect(Socket, &QTcpSocket::connected, WaitingLoop, &QEventLoop::quit);
  connect(Socket, &QTcpSocket::disconnected, WaitingLoop, &QEventLoop::quit);
  connect(Socket, &QTcpSocket::errorOccurred, WaitingLoop, &QEventLoop::quit);
  connect(WaitTimer, &QTimer::timeout, WaitingLoop, &QEventLoop::quit);
  connect(this, &PersoClient::stopResponseWaiting, WaitingLoop,
          &QEventLoop::quit);
}

void PersoClient::createSocket() {
  Socket = new QTcpSocket(this);
  connect(Socket, &QTcpSocket::connected, this,
          &PersoClient::on_SocketConnected_slot);
  connect(Socket, &QTcpSocket::disconnected, this,
          &PersoClient::on_SocketDisconnected_slot);
  connect(Socket, &QTcpSocket::readyRead, this,
          &PersoClient::on_SocketReadyRead_slot);
  connect(Socket, &QTcpSocket::errorOccurred, this,
          &PersoClient::on_SocketError_slot);
}

void PersoClient::createResponseHandlers() {
  ResponseHandlers.insert("echo", &PersoClient::processEcho);
  ResponseHandlers.insert("authorization", &PersoClient::processAuthorization);
  ResponseHandlers.insert("transponder_release",
                          &PersoClient::processTransponderRelease);
  ResponseHandlers.insert("transponder_release_confirm",
                          &PersoClient::processTransponderReleaseConfirm);
  ResponseHandlers.insert("transponder_rerelease",
                          &PersoClient::processTransponderRerelease);
  ResponseHandlers.insert("transponder_rerelease_confirm",
                          &PersoClient::processTransponderRereleaseConfirm);
  ResponseHandlers.insert("production_line_rollback",
                          &PersoClient::processProductionLineRollback);
  ResponseHandlers.insert("print_box_sticker",
                          &PersoClient::processPrintBoxSticker);
  ResponseHandlers.insert("print_last_box_sticker",
                          &PersoClient::processPrintLastBoxSticker);
  ResponseHandlers.insert("print_pallet_sticker",
                          &PersoClient::processPrintPalletSticker);
  ResponseHandlers.insert("print_last_pallet_sticker",
                          &PersoClient::processPrintLastPalletSticker);
}

void PersoClient::createResponseTemplates() {
  std::shared_ptr<QVector<QString>> echoSyntax(new QVector<QString>());
  echoSyntax->append("data");
  echoSyntax->append("return_status");
  ResponseTemplates.insert("echo", echoSyntax);

  std::shared_ptr<QVector<QString>> authorizationSyntax(new QVector<QString>());
  authorizationSyntax->append("access");
  authorizationSyntax->append("return_status");
  ResponseTemplates.insert("authorization", authorizationSyntax);

  std::shared_ptr<QVector<QString>> transponderReleaseSyntax(
      new QVector<QString>());
  transponderReleaseSyntax->append("firmware");
  transponderReleaseSyntax->append("sn");
  transponderReleaseSyntax->append("pan");
  transponderReleaseSyntax->append("box_id");
  transponderReleaseSyntax->append("pallet_id");
  transponderReleaseSyntax->append("order_id");
  transponderReleaseSyntax->append("issuer_name");
  transponderReleaseSyntax->append("transponder_model");
  transponderReleaseSyntax->append("return_status");
  ResponseTemplates.insert("transponder_release", transponderReleaseSyntax);

  std::shared_ptr<QVector<QString>> transponderReleaseConfirmSyntax(
      new QVector<QString>());
  transponderReleaseConfirmSyntax->append("return_status");
  ResponseTemplates.insert("transponder_release_confirm",
                           transponderReleaseConfirmSyntax);

  std::shared_ptr<QVector<QString>> transponderRereleaseSyntax(
      new QVector<QString>());
  transponderRereleaseSyntax->append("firmware");
  transponderRereleaseSyntax->append("sn");
  transponderRereleaseSyntax->append("pan");
  transponderRereleaseSyntax->append("box_id");
  transponderRereleaseSyntax->append("pallet_id");
  transponderRereleaseSyntax->append("order_id");
  transponderRereleaseSyntax->append("issuer_name");
  transponderRereleaseSyntax->append("transponder_model");
  transponderRereleaseSyntax->append("return_status");
  ResponseTemplates.insert("transponder_rerelease", transponderRereleaseSyntax);

  std::shared_ptr<QVector<QString>> transponderRereleaseConfirmSyntax(
      new QVector<QString>());
  transponderRereleaseConfirmSyntax->append("return_status");
  ResponseTemplates.insert("transponder_rerelease_confirm",
                           transponderReleaseConfirmSyntax);

  std::shared_ptr<QVector<QString>> productionLineRollbackSyntax(
      new QVector<QString>());
  productionLineRollbackSyntax->append("return_status");
  ResponseTemplates.insert("production_line_rollback",
                           productionLineRollbackSyntax);

  std::shared_ptr<QVector<QString>> printBoxStickerSyntax(
      new QVector<QString>());
  printBoxStickerSyntax->append("return_status");
  ResponseTemplates.insert("print_box_sticker", printBoxStickerSyntax);

  std::shared_ptr<QVector<QString>> printLastBoxStickerSyntax(
      new QVector<QString>());
  printLastBoxStickerSyntax->append("return_status");
  ResponseTemplates.insert("print_last_box_sticker", printLastBoxStickerSyntax);

  std::shared_ptr<QVector<QString>> printPalletStickerSyntax(
      new QVector<QString>());
  printPalletStickerSyntax->append("return_status");
  ResponseTemplates.insert("print_pallet_sticker", printPalletStickerSyntax);

  std::shared_ptr<QVector<QString>> printLastPalletStickerSyntax(
      new QVector<QString>());
  printLastPalletStickerSyntax->append("return_status");
  ResponseTemplates.insert("print_last_pallet_sticker",
                           printLastPalletStickerSyntax);
}

void PersoClient::createServerStatusMatchTable() {
  ServerStatusMatchTable.insert("1", CommandSyntaxError);
  ServerStatusMatchTable.insert("2", DatabaseError);
  ServerStatusMatchTable.insert("3", TransponderNotFound);
  ServerStatusMatchTable.insert("4", TransponderNotReleasedEarlier);
  ServerStatusMatchTable.insert("5", AwaitingConfirmationError);
  ServerStatusMatchTable.insert("6", IdenticalUcidError);
  ServerStatusMatchTable.insert("7", ProductionLineMissed);
  ServerStatusMatchTable.insert("8", ProductionLineNotActive);
  ServerStatusMatchTable.insert("9", CurrentOrderRunOut);
  ServerStatusMatchTable.insert("10", CurrentOrderAssembled);
  ServerStatusMatchTable.insert("11", ProductionLineRollbackLimitError);
  ServerStatusMatchTable.insert("12", BoxStickerPrintError);
  ServerStatusMatchTable.insert("13", PalletStickerPrintError);
  ServerStatusMatchTable.insert("14", NextTransponderNotFound);
  ServerStatusMatchTable.insert("15", StartBoxAssemblingError);
  ServerStatusMatchTable.insert("16", StartPalletAssemblingError);
}

void PersoClient::on_SocketConnected_slot() {
  sendLog("Соединение с сервером персонализации установлено. ");
}

void PersoClient::on_SocketDisconnected_slot() {
  sendLog("Соединение с сервером персонализации отключено. ");
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
      WaitTimer->start();
      return;
    }
    // Сохраняем размер блока данных
    deserializator >> ReceivedDataBlockSize;

    sendLog(QString("Размер ожидаемого блока данных: %1.")
                .arg(QString::number(ReceivedDataBlockSize)));

    // Если размер блока данных слишком большой, то весь блок отбрасывается
    if (ReceivedDataBlockSize > DATA_BLOCK_MAX_SIZE) {
      sendLog("Размер блока данных слишком большой. Сброс. ");
      // Останавливаем цикл ожидания
      emit stopResponseWaiting();
      ReceivedDataBlockSize = 0;
    }
  }

  sendLog(QString("Размер принятых данных: %1. ")
              .arg(QString::number(Socket->bytesAvailable())));

  // Дожидаемся пока весь блок данных придет целиком
  if (Socket->bytesAvailable() < ReceivedDataBlockSize) {
    sendLog("Блок получен не целиком. Ожидается прием следующих частей. ");
    // Перезапускаем таймер ожидания для следующих частей
    WaitTimer->start();
    return;
  }

  // Если блок был получен целиком, то осуществляем его дессериализацию
  deserializator >> ReceivedDataBlock;

  // Останавливаем цикл ожидания
  emit stopResponseWaiting();
}

void PersoClient::on_SocketError_slot(
    QAbstractSocket::SocketError socketError) {
  Socket->abort();
  sendLog(QString("Ошибка сети: %1. %2.")
              .arg(QString::number(socketError), Socket->errorString()));
}

void PersoClient::on_WaitTimerTimeout_slot() {
  Socket->abort();
  ReceivedDataBlock.clear();
  sendLog("Время ожидания вышло. ");
}
