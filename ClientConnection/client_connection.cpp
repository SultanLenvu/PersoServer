#include <QSettings>

#include "General/definitions.h"
#include "Log/log_system.h"
#include "box_sticker_print_command.h"
#include "client_connection.h"
#include "echo_comand.h"
#include "last_box_sticker_print_command.h"
#include "last_pallet_sticker_print_command.h"
#include "log_in_command.h"
#include "log_out_command.h"
#include "pallet_sticker_print_command.h"
#include "release_command.h"
#include "release_confirm_command.h"
#include "rerelease_command.h"
#include "rerelease_confirm_command.h"
#include "rollback_command.h"
#include "update_command.h"

ClientConnection::ClientConnection(const QString& name,
                                   uint32_t id,
                                   qintptr socketDescriptor)
    : AbstractClientConnection(name) {
  Id = id;
  ReceivedDataBlockSize = 0;

  // Загружаем настройки
  loadSettings();

  // Создаем сокет в соответствии с системным дескриптором
  createSocket(socketDescriptor);

  // Создаем таймер экспирации подключения
  createExpirationTimer();

  // Создаем таймер для приема блоков данных частями
  createDataBlockWaitTimer();

  // Создаем команды
  createCommands();
}

ClientConnection::~ClientConnection() {
  sendLog(QString("Клиент %1 удален. ").arg(QString::number(Id)));
}

size_t ClientConnection::getId() const {
  return Id;
}

bool ClientConnection::isAuthorised() const {
  return Authorized;
}

const QString& ClientConnection::getLogin() const {
  return Login;
}

const QString& ClientConnection::getPassword() const {
  return Password;
}

void ClientConnection::loadSettings() {
  QSettings settings;

  IdleExpirationTime =
      settings.value("perso_client/idle_expiration_time").toInt();
}

void ClientConnection::sendLog(const QString& log) const {
  LogSystem::instance()->generate(objectName() + " - " + log);
}

void ClientConnection::createTransmittedDataBlock() {
  // Вызов обработчика
  Commands.value(CommandData.value("command_name").toString())
      ->generateResponse(ResponseData);
  QJsonDocument responseDocument(ResponseData);

  sendLog("Формирование блока данных для ответа на команду. ");
  sendLog(QString("Размер ответа: %1.")
              .arg(QString::number(responseDocument.toJson().size())));
  sendLog(
      QString("Содержание ответа: %1").arg(QString(responseDocument.toJson())));

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

bool ClientConnection::processReceivedDataBlock(void) {
  QJsonParseError status;
  QJsonDocument requestDocument =
      QJsonDocument::fromJson(ReceivedDataBlock, &status);

  // Если пришел некорректный JSON
  if (status.error != QJsonParseError::NoError) {
    sendLog("Ошибка парсинга JSON команды. Сброс. ");
    return false;
  }

  // Выделяем список пар ключ-значение из JSON-файла
  sendLog("Обработка полученного блока данных. ");
  CommandData = requestDocument.object();

  // Синтаксическая проверка
  if (CommandData.value("command_name").isUndefined()) {
    sendLog("Получена синтаксическая ошибка: отсутствует название команды. ");
    return false;
  }

  // Проверка имени команды
  if (!Commands.contains(CommandData.value("command_name").toString())) {
    sendLog(QString("Получена неизвестная команда: %1. ")
                .arg(CommandData.value("command_name").toString()));
    return false;
  }
  sendLog(QString("Получена команда: %1. ")
              .arg(CommandData.value("command_name").toString()));

  // Вызов обработчика
  Commands.value(CommandData.value("command_name").toString())
      ->process(CommandData);

  return true;
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
  ExpirationTimer = std::unique_ptr<QTimer>(new QTimer());
  ExpirationTimer->setInterval(IdleExpirationTime);
  // Если время подключения вышло, то вызываем соответствующий обработчик
  connect(ExpirationTimer.get(), &QTimer::timeout, this,
          &ClientConnection::expirationTimerTimeout_slot);
  // Если время подключения вышло, то останавливаем таймер экспирации
  connect(ExpirationTimer.get(), &QTimer::timeout, ExpirationTimer.get(),
          &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::errorOccurred, ExpirationTimer.get(),
          &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::disconnected, ExpirationTimer.get(),
          &QTimer::stop);

  // Запускаем таймер экспирации
  ExpirationTimer->start();
}

void ClientConnection::createDataBlockWaitTimer() {
  // Таймер ожидания для приема блоков данных по частям
  DataBlockWaitTimer = std::unique_ptr<QTimer>(new QTimer());
  DataBlockWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  // Если время ожидания вышло, то вызываем соответствующий обработчик
  connect(DataBlockWaitTimer.get(), &QTimer::timeout, this,
          &ClientConnection::dataBlockWaitTimerTimeout_slot);
  // Если пришли данные, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::readyRead, DataBlockWaitTimer.get(),
          &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::disconnected, DataBlockWaitTimer.get(),
          &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер ожидания
  connect(Socket, &QTcpSocket::errorOccurred, DataBlockWaitTimer.get(),
          &QTimer::stop);
  // Если время подключения вышло, то таймер ожидания останавливается
  connect(ExpirationTimer.get(), &QTimer::timeout, DataBlockWaitTimer.get(),
          &QTimer::stop);
}

void ClientConnection::createCommands() {
  Commands.insert("echo", std::shared_ptr<AbstractClientCommand>(
                              new EchoCommand("EchoCommand")));

  Commands.insert("log_in", std::shared_ptr<AbstractClientCommand>(
                                new LogInCommand("LogInCommand")));
  connect(dynamic_cast<LogInCommand*>(Commands.value("log_in").get()),
          &LogInCommand::authorized, this, &ClientConnection::authorized_slot);

  Commands.insert("log_out", std::shared_ptr<AbstractClientCommand>(
                                 new LogOutCommand("LogOutCommand")));
  connect(dynamic_cast<LogOutCommand*>(Commands.value("log_out").get()),
          &LogOutCommand::deauthorized, this,
          &ClientConnection::deauthorized_slot);

  Commands.insert("update", std::shared_ptr<AbstractClientCommand>(
                                new UpdateCommand("UpdateCommand")));

  Commands.insert("release", std::shared_ptr<AbstractClientCommand>(
                                 new ReleaseCommand("ReleaseCommand")));
  Commands.insert("release_confirm",
                  std::shared_ptr<AbstractClientCommand>(
                      new ReleaseConfirmCommand("ReleaseConfirmCommand")));
  Commands.insert("rerelease", std::shared_ptr<AbstractClientCommand>(
                                   new RereleaseCommand("RereleaseCommand")));
  Commands.insert("rerelease_confirm",
                  std::shared_ptr<AbstractClientCommand>(
                      new RereleaseConfirmCommand("RereleaseConfirmCommand")));
  Commands.insert("rollback", std::unique_ptr<AbstractClientCommand>(
                                  new RollbackCommand("RollbackCommand")));

  Commands.insert("box_sticker_print",
                  std::shared_ptr<AbstractClientCommand>(
                      new BoxStickerPrintCommand("BoxStickerPrintCommand")));
  Commands.insert(
      "last_box_sticker_print",
      std::shared_ptr<AbstractClientCommand>(
          new LastBoxStickerPrintCommand("LastBoxStickerPrintCommand")));
  Commands.insert(
      "pallet_sticker_print",
      std::shared_ptr<AbstractClientCommand>(
          new PalletStickerPrintCommand("PalletStickerPrintCommand")));
  Commands.insert(
      "last_pallet_sticker_print",
      std::shared_ptr<AbstractClientCommand>(
          new LastPalletStickerPrintCommand("LastPalletStickerPrintCommand")));
}

void ClientConnection::socketReadyRead_slot() {
  QDataStream deserializator(Socket);  // Дессериализатор
  deserializator.setVersion(
      QDataStream::Qt_6_5);  // Настраиваем версию десериализатора

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
  sendLog("Блок полученных данных: " + ReceivedDataBlock);

  // Осуществляем обработку полученного блока данных
  if (!processReceivedDataBlock()) {
    return;
  }

  // Создаем блок данных для ответа на команду
  createTransmittedDataBlock();

  // Отправляем сформированный блок данных
  transmitDataBlock();

  // Очистка
  Commands.value(CommandData.value("command_name").toString())->reset();
  CommandData = QJsonObject();
  ResponseData = QJsonObject();
}

void ClientConnection::socketDisconnected_slot() {
  sendLog("Сетевое соединение оборвалось. ");

  // Отправляем сигналы об отключении клиента
  if (Authorized) {
    StringDictionary param;
    ReturnStatus status;
    param.insert("login", Login);
    param.insert("password", Password);
    emit logOut_signal(param, status);
  }

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
  DataBlockWaitTimer->stop();
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}

void ClientConnection::authorized_slot(const QString& login,
                                       const QString& password) {
  sendLog("Клиент авторизовался. ");
  ExpirationTimer->stop();

  Login = login;
  Password = password;
}

void ClientConnection::deauthorized_slot() {
  sendLog("Клиент деавторизировался. ");
  Login.clear();
  Password.clear();

  ExpirationTimer->start();
}
