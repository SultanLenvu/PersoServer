#include <QSettings>

#include "client_connection.h"
#include "complete_current_box_command.h"
#include "confirm_transponder_release_command.h"
#include "confirm_transponder_rerelease_command.h"
#include "definitions.h"
#include "echo_comand.h"
#include "get_current_box_data_command.h"
#include "get_current_transponder_data_command.h"
#include "get_transponder_data_command.h"
#include "log_in_command.h"
#include "log_out_command.h"
#include "print_box_sticker_command.h"
#include "print_last_box_sticker_command.h"
#include "print_last_pallet_sticker_command.h"
#include "print_pallet_sticker_command.h"
#include "refund_current_box_command.h"
#include "release_transponder_command.h"
#include "request_box_command.h"
#include "rerelease_transponder_command.h"
#include "rollback_transponder_command.h"

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

  // Создаем контекст
  createContext();
}

ClientConnection::~ClientConnection() {
  sendLog(QString("Клиент %1 удален. ").arg(QString::number(Id)));
}

size_t ClientConnection::getId() const {
  return Id;
}

void ClientConnection::loadSettings() {
  QSettings settings;

  IdleExpirationTime =
      settings.value("perso_client/unauthorized_access_expiration_time").toInt();
}

void ClientConnection::sendLog(const QString& log) {
  emit const_cast<ClientConnection*>(this)->logging(objectName() + " - " + log);
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
  serializator.setVersion(QDataStream::Qt_6_5);

  // Формируем единый блок данных для отправки
  serializator << uint32_t(0) << responseDocument.toJson();
  serializator.device()->seek(0);
  serializator << uint32_t(TransmittedDataBlock.size() - sizeof(uint32_t));
}

void ClientConnection::transmitDataBlock() {
  // Если размер блок не превышает максимального размера данных для
  // единоразовой передачи
  if (TransmittedDataBlock.size() < ONETIME_TRANSMIT_DATA_SIZE) {
    // Отправляем блок данных
    Socket->write(TransmittedDataBlock);
    return;
  }

  // В противном случае дробим блок данных на части и последовательно
  // отправляем
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
  Socket = std::unique_ptr<QTcpSocket>(new QTcpSocket());
  Socket->setSocketDescriptor(socketDescriptor);

  connect(Socket.get(), &QTcpSocket::readyRead, this,
          &ClientConnection::socketReadyRead_slot);
  connect(Socket.get(), &QTcpSocket::disconnected, this,
          &ClientConnection::socketDisconnected_slot);
  connect(
      Socket.get(),
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
  connect(Socket.get(), &QTcpSocket::errorOccurred, ExpirationTimer.get(),
          &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер экспирации
  connect(Socket.get(), &QTcpSocket::disconnected, ExpirationTimer.get(),
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
  connect(Socket.get(), &QTcpSocket::readyRead, DataBlockWaitTimer.get(),
          &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер ожидания
  connect(Socket.get(), &QTcpSocket::disconnected, DataBlockWaitTimer.get(),
          &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер ожидания
  connect(Socket.get(), &QTcpSocket::errorOccurred, DataBlockWaitTimer.get(),
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

  Commands.insert("request_box",
                  std::shared_ptr<AbstractClientCommand>(
                      new RequestBoxCommand("RequestBoxCommand")));
  Commands.insert(
      "get_current_box_data",
      std::shared_ptr<AbstractClientCommand>(
          new GetCurrentBoxDataCommand("GetCurrentBoxDataCommand")));
  Commands.insert(
      "complete_box",
      std::shared_ptr<AbstractClientCommand>(
          new CompleteCurrentBoxCommand("CompleteCurrentBoxCommand")));
  Commands.insert("refund_current_box",
                  std::shared_ptr<AbstractClientCommand>(
                      new RefundCurrentBoxCommand("RefundCurrentBoxCommand")));

  Commands.insert(
      "release_transponder",
      std::shared_ptr<AbstractClientCommand>(
          new ReleaseTransponderCommand("ReleaseTransponderCommand")));
  Commands.insert("confirm_transponder_release",
                  std::shared_ptr<AbstractClientCommand>(
                      new ConfirmTransponderReleaseCommand(
                          "ConfirmTransponderReleaseCommand")));
  Commands.insert(
      "rerelease_transponder",
      std::shared_ptr<AbstractClientCommand>(
          new RereleaseTransponderCommand("RereleaseTransponderCommand")));
  Commands.insert("confirm_transponder_rerelease",
                  std::shared_ptr<AbstractClientCommand>(
                      new ConfirmTransponderRereleaseCommand(
                          "ConfirmTransponderRereleaseCommand")));
  Commands.insert(
      "rollback_transponder",
      std::unique_ptr<AbstractClientCommand>(
          new RollbackTransponderCommand("RollbackTransponderCommand")));
  Commands.insert("get_current_transponder_data",
                  std::unique_ptr<AbstractClientCommand>(
                      new GetCurrentTransponderDataCommand(
                          "GetCurrentTransponderDataCommand")));
  Commands.insert(
      "get_transponder_data",
      std::unique_ptr<AbstractClientCommand>(
          new GetTransponderDataCommand("GetTransponderDataCommand")));

  Commands.insert("print_box_sticker",
                  std::shared_ptr<AbstractClientCommand>(
                      new BoxStickerPrintCommand("BoxStickerPrintCommand")));
  Commands.insert(
      "print_last_box_sticker",
      std::shared_ptr<AbstractClientCommand>(
          new PrintLastBoxStickerCommand("PrintLastBoxStickerCommand")));
  Commands.insert(
      "print_pallet_sticker",
      std::shared_ptr<AbstractClientCommand>(
          new PrintPalletStickerCommand("PrintPalletStickerCommand")));
  Commands.insert(
      "print_last_pallet_sticker",
      std::shared_ptr<AbstractClientCommand>(
          new PrintLastPalletStickerCommand("PrintLastPalletStickerCommand")));
}

void ClientConnection::createContext() {
  Context = std::shared_ptr<ProductionContext>(new ProductionContext());

  for (auto it = Commands.begin(); it != Commands.end(); ++it) {
    it.value()->setContext(Context);
  }
}

void ClientConnection::socketReadyRead_slot() {
  QDataStream deserializator(Socket.get());  // Дессериализатор
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

    //    ReceivedDataBlock = Socket->readAll();

    sendLog(QString("Размер принимаемого блока данных: %1.")
                .arg(QString::number(ReceivedDataBlockSize)));

    // Если размер блока данных слишком большой, то весь блок отбрасывается
    if (ReceivedDataBlockSize > DATA_BLOCK_MAX_SIZE) {
      sendLog("Размер блока данных слишком большой. Сброс. ");
      ReceivedDataBlockSize = 0;
      ReceivedDataBlock.clear();
    }
  }

  sendLog(QString("Размер полученного блока данных: %1. ")
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
  sendLog(QString("Полученный блок данных: %1").arg(ReceivedDataBlock));

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
  sendLog("Экспирация времени неавторизированного подключения. ");

  // Закрываем соединение
  Socket->close();
}

void ClientConnection::dataBlockWaitTimerTimeout_slot() {
  sendLog("Время ожидания вышло. Блок данных сбрасывается. ");
  DataBlockWaitTimer->stop();
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}

void ClientConnection::authorized_slot() {
  sendLog("Клиент авторизовался. ");
  ExpirationTimer->stop();
}

void ClientConnection::deauthorized_slot() {
  sendLog("Клиент деавторизировался. ");
  ExpirationTimer->start();
}
