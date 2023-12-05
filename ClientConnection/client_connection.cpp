#include <QSettings>

#include "General/definitions.h"
#include "Log/log_system.h"
#include "box_sticker_print_command.h"
#include "client_connection.h"
#include "echo_comand.h"
#include "firmware_generation_system.h"
#include "get_current_context_command.h"
#include "last_box_sticker_print_command.h"
#include "last_pallet_sticker_print_command.h"
#include "launch_command.h"
#include "pallet_sticker_print_command.h"
#include "transponder_release_command.h"
#include "transponder_release_confirm_command.h"
#include "transponder_rerelease_command.h"
#include "transponder_rerelease_confirm_command.h"

ClientConnection::ClientConnection(const QString& name,
                                   uint32_t id,
                                   qintptr socketDescriptor)
    : AbstractClientConnection(name) {
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

  // Создаем команды
  createCommands();
}

ClientConnection::~ClientConnection() {
  sendLog(QString("Клиент %1 удален. ").arg(QString::number(Id)));
}

size_t ClientConnection::getId() const {
  return Id;
}

void ClientConnection::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
  ExtendedLogEnable = settings.value("log_system/extended_enable").toBool();

  MaximumConnectionTime =
      settings.value("perso_client/connection_max_duration").toInt();
}

void ClientConnection::sendLog(const QString& log) {
  if (LogEnable) {
    emit const_cast<ClientConnection*>(this)->logging("ClientConnection - " +
                                                      log);
  }
}

void ClientConnection::createTransmittedDataBlock() {
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

void ClientConnection::transmitDataBlock() {
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

void ClientConnection::processReceivedDataBlock(void) {
  QJsonParseError status;
  QJsonDocument requestDocument =
      QJsonDocument::fromJson(ReceivedDataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    sendLog("Ошибка парсинга JSON команды. Сброс. ");
    return;
  }

  // Выделяем список пар ключ-значение из JSON-файла
  sendLog("Обработка полученного блока данных. ");
  CurrentCommand = requestDocument.object();

  // Синтаксическая проверка
  if (CurrentCommand.value("command_name").isUndefined()) {
    sendLog("Получена синтаксическая ошибка: отсутствует название команды. ");
    return;
  }

  // Проверка имени команды
  if (!CommandHandlers.value(CurrentCommand.value("command_name").toString())) {
    sendLog(QString("Получена неизвестная команда: %1. ")
                .arg(CurrentCommand.value("command_name").toString()));
    return;
  }

  // Заголовок ответа на команду
  sendLog(QString("Получена команда: %1. ")
              .arg(CurrentCommand.value("command_name").toString()));
  CurrentResponse["response_name"] =
      CurrentCommand.value("command_name").toString();

  // Синтаксическая проверка команды
  QVector<QString>* currentTemplate =
      CommandTemplates.value(CurrentCommand.value("command_name").toString())
          .get();
  QVector<QString>::iterator it;
  for (it = currentTemplate->begin(); it != currentTemplate->end(); it++) {
    if (!CurrentCommand.contains(*it)) {
      processSyntaxError();
      return;
    }
  }

  // Вызов обработчика
  (this->*CommandHandlers.value(
              CurrentCommand.value("command_name").toString()))();
}

void ClientConnection::processSyntaxError() {
  sendLog(QString("Получена синтаксическая ошибка в команде '%1'. ")
              .arg(CurrentCommand.value("command_name").toString()));

  CurrentResponse["return_status"] = QString::number(CommandSyntaxError);
}

void ClientConnection::processAuthorization() {
  sendLog("Выполнение команды authorization. ");
  QHash<QString, QString> authorizationParameters;
  TransponderReleaseSystem::ReturnStatus ret;

  // Логика
  authorizationParameters.insert("login",
                                 CurrentCommand.value("login").toString());
  authorizationParameters.insert("password",
                                 CurrentCommand.value("password").toString());
  emit authorize_signal(&authorizationParameters, &ret);

  CurrentResponse["return_status"] = QString::number(ret);

  //  if (ret == TransponderReleaseSystem::Completed) {
  //    CurrentResponse["access"] = "allowed";
  //  } else if (ret == TransponderReleaseSystem::ProductionLineNotActive) {
  //    CurrentResponse["access"] = "not_active";
  //  } else if (ret == TransponderReleaseSystem::ProductionLineMissed) {
  //    CurrentResponse["access"] = "not_exist";
  //  } else {
  //    CurrentResponse["access"] = "denied";
  //  }
  //  CurrentResponse["return_status"] = QString::number(NoError);
}

void ClientConnection::processTransponderRelease() {
  sendLog("Выполнение команды transponder_release. ");
  QHash<QString, QString> releaseParameters;
  TransponderReleaseSystem::ReturnStatus ret;
  QHash<QString, QString> seed;
  QHash<QString, QString> data;

  // Выпуск транспондера
  releaseParameters.insert("login", CurrentCommand.value("login").toString());
  releaseParameters.insert("password",
                           CurrentCommand.value("password").toString());
  emit release_signal(&releaseParameters, &seed, &data, &ret);

  if (ret != TransponderReleaseSystem::Completed) {
    sendLog("Получена ошибка при выпуске транспондера. ");
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(ret));
    return;
  }

  sendLog("Генерация прошивки транспондера. ");
  QByteArray firmware;
  Generator->generate(&seed, &firmware);
  CurrentResponse["firmware"] = QString::fromUtf8(firmware.toBase64());
  CurrentResponse["sn"] = data.value("sn");
  CurrentResponse["pan"] = data.value("pan");
  CurrentResponse["box_id"] = data.value("box_id");
  CurrentResponse["pallet_id"] = data.value("pallet_id");
  CurrentResponse["order_id"] = data.value("order_id");
  CurrentResponse["issuer_name"] = data.value("issuer_name");
  CurrentResponse["transponder_model"] = data.value("transponder_model");
  CurrentResponse["return_status"] = QString::number(NoError);
}

void ClientConnection::processTransponderReleaseConfirm() {
  sendLog("Выполнение команды transponder_release_confirm. ");
  QHash<QString, QString> confirmParameters;
  TransponderReleaseSystem::ReturnStatus ret;

  // Подтверждение выпуска транспондера
  confirmParameters.insert("login", CurrentCommand.value("login").toString());
  confirmParameters.insert("password",
                           CurrentCommand.value("password").toString());
  confirmParameters.insert("ucid", CurrentCommand.value("ucid").toString());
  emit confirmRelease_signal(&confirmParameters, &ret);

  if (ret != TransponderReleaseSystem::Completed) {
    sendLog("Получена ошибка при подтверждении выпуска транспондера. ");
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(ret));
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ClientConnection::processTransponderRerelease() {
  sendLog("Выполнение команды transponder_rerelease. ");
  QHash<QString, QString> rereleaseParameters;
  TransponderReleaseSystem::ReturnStatus ret;
  QHash<QString, QString> seed;
  QHash<QString, QString> data;

  // Перевыпуск транспондера
  rereleaseParameters.insert(
      "personal_account_number",
      CurrentCommand.value("pan").toString().leftJustified(FULL_PAN_CHAR_LENGTH,
                                                           QChar('F')));
  emit rerelease_signal(&rereleaseParameters, &seed, &data, &ret);

  if (ret != TransponderReleaseSystem::Completed) {
    sendLog("Получена ошибка при перевыпуске транспондера. ");
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(ret));
    return;
  }

  sendLog("Генерация прошивки транспондера. ");
  QByteArray firmware;
  Generator->generate(&seed, &firmware);
  CurrentResponse["firmware"] = QString::fromUtf8(firmware.toBase64());
  CurrentResponse["sn"] = data.value("sn");
  CurrentResponse["pan"] = data.value("pan");
  CurrentResponse["box_id"] = data.value("box_id");
  CurrentResponse["pallet_id"] = data.value("pallet_id");
  CurrentResponse["order_id"] = data.value("order_id");
  CurrentResponse["issuer_name"] = data.value("issuer_name");
  CurrentResponse["transponder_model"] = data.value("transponder_model");
  CurrentResponse["return_status"] = QString::number(NoError);
}

void ClientConnection::processTransponderRereleaseConfirm() {
  sendLog("Выполнение команды transponder_rerelease_confirm. ");
  QHash<QString, QString> confirmParameters;
  TransponderReleaseSystem::ReturnStatus ret;

  // Подтверждение перевыпуска транспондера
  confirmParameters.insert("personal_account_number",
                           CurrentCommand.value("pan").toString().leftJustified(
                               FULL_PAN_CHAR_LENGTH, QChar('F')));
  confirmParameters.insert("ucid", CurrentCommand.value("ucid").toString());
  emit confirmRerelease_signal(&confirmParameters, &ret);

  if (ret != TransponderReleaseSystem::Completed) {
    sendLog("Получена ошибка при подтверждении перевыпуска транспондера. ");
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(ret));
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ClientConnection::createSocket(qintptr socketDescriptor) {
  Socket = new QTcpSocket(this);
  Socket->setSocketDescriptor(socketDescriptor);

  connect(Socket, &QTcpSocket::readyRead, this,
          &ClientConnection::socketReadyRead_slot);
  connect(Socket, &QTcpSocket::disconnected, this,
          &ClientConnection::socketDisconnected_slot);
  connect(
      Socket,
      QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
      this, &ClientConnection::socketError_slot);
}

void ClientConnection::createExpirationTimer() {
  // Таймер для отсчета времени экспирации
  ExpirationTimer = new QTimer(this);
  ExpirationTimer->setInterval(MaximumConnectionTime);
  // Если время подключения вышло, то вызываем соответствующий обработчик
  connect(ExpirationTimer, &QTimer::timeout, this,
          &ClientConnection::expirationTimerTimeout_slot);
  // Если время подключения вышло, то останавливаем таймер экспирации
  connect(ExpirationTimer, &QTimer::timeout, ExpirationTimer, &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::errorOccurred, ExpirationTimer, &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::disconnected, ExpirationTimer, &QTimer::stop);

  // Запускаем таймер экспирации
  ExpirationTimer->start();
}

void ClientConnection::createDataBlockWaitTimer() {
  // Таймер ожидания для приема блоков данных по частям
  DataBlockWaitTimer = new QTimer(this);
  DataBlockWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  // Если время ожидания вышло, то вызываем соответствующий обработчик
  connect(DataBlockWaitTimer, &QTimer::timeout, this,
          &ClientConnection::dataBlockWaitTimerTimeout_slot);
  // Если время ожидания вышло, то останавливаем таймер ожидания
  connect(DataBlockWaitTimer, &QTimer::timeout, DataBlockWaitTimer,
          &QTimer::stop);
  // Если пришли данные, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::readyRead, DataBlockWaitTimer, &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::disconnected, DataBlockWaitTimer, &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::errorOccurred, DataBlockWaitTimer,
          &QTimer::stop);
  // Если время подключения вышло, то таймер ожидания останавливается
  connect(ExpirationTimer, &QTimer::timeout, DataBlockWaitTimer, &QTimer::stop);
}

void ClientConnection::createGenerator() {
  Generator = std::unique_ptr<FirmwareGenerationSystem>(
      new FirmwareGenerationSystem("FirmwareGenerationSystem"));

  connect(Generator, &FirmwareGenerationSystem::logging, LogSystem::instance(),
          &LogSystem::generate);
}

void ClientConnection::createCommands() {
  Commands.insert("echo", std::unique_ptr(new EchoCommand("EchoCommand")))
}

void ClientConnection::socketReadyRead_slot() {
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

void ClientConnection::socketDisconnected_slot() {
  sendLog("Отключился. ");

  // Отправляем сигнал об отключении клиента
  emit disconnected();
}

void ClientConnection::socketError_slot(
    QAbstractSocket::SocketError socketError) {
  // Если клиент отключился не самостоятельно
  if (socketError != 1) {
    sendLog(QString("Ошибка сети: %1. %2.")
                .arg(QString::number(socketError), Socket->errorString()));
    Socket->close();
  }
}

void ClientConnection::expirationTimerTimeout_slot() {
  sendLog("Экспирация времени подключения. ");

  // Закрываем соединение
  Socket->close();
}

void ClientConnection::dataBlockWaitTimerTimeout_slot() {
  sendLog("Время ожидания вышло. Блок данных сбрасывается. ");
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}
