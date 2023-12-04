#include "production_line_connection.h"

ProductionLineConnection::ProductionLineConnection(const QString& name,
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

  // Создаем шаблоны команд
  createCommandTemplates();
  createCommandHandlers();
  createServerStatusMatchTable();
}

ProductionLineConnection::~ProductionLineConnection() {
  sendLog(QString("Клиент %1 удален. ").arg(QString::number(Id)));
}

size_t ProductionLineConnection::getId() const {
  return Id;
}

void ProductionLineConnection::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
  ExtendedLogEnable = settings.value("log_system/extended_enable").toBool();

  MaximumConnectionTime =
      settings.value("perso_client/connection_max_duration").toInt();
}

void ProductionLineConnection::sendLog(const QString& log) {
  if (LogEnable) {
    emit const_cast<ProductionLineConnection*>(this)->logging(
        "ProductionLineConnection - " + log);
  }
}

void ProductionLineConnection::createTransmittedDataBlock() {
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

void ProductionLineConnection::transmitDataBlock() {
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

void ProductionLineConnection::processReceivedDataBlock(void) {
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

void ProductionLineConnection::processSyntaxError() {
  sendLog(QString("Получена синтаксическая ошибка в команде '%1'. ")
              .arg(CurrentCommand.value("command_name").toString()));

  CurrentResponse["return_status"] = QString::number(CommandSyntaxError);
}

void ProductionLineConnection::processEcho() {
  sendLog("Выполнение команды Echo. ");

  // Формирование ответа
  CurrentResponse["data"] = CurrentCommand.value("data");
  CurrentResponse["return_status"] = QString::number(NoError);
}

void ProductionLineConnection::processAuthorization() {
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

void ProductionLineConnection::processTransponderRelease() {
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

void ProductionLineConnection::processTransponderReleaseConfirm() {
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

void ProductionLineConnection::processTransponderRerelease() {
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

void ProductionLineConnection::processTransponderRereleaseConfirm() {
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

void ProductionLineConnection::processProductionLineRollback() {
  sendLog("Выполнение команды production_line_rollback. ");
  TransponderReleaseSystem::ReturnStatus ret;

  // Печать стикера для бокса
  QHash<QString, QString> data;
  data.insert("login", CurrentCommand.value("login").toString());
  data.insert("password", CurrentCommand.value("password").toString());

  emit productionLineRollback_signal(&data, &ret);

  if (ret != TransponderReleaseSystem::Completed) {
    sendLog("Получена ошибка при откате производственной линии. ");
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(ret));
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ProductionLineConnection::processPrintBoxSticker() {
  sendLog("Выполнение команды print_box_sticker. ");

  QHash<QString, QString> parameters;
  QHash<QString, QString> boxData;
  AbstractStickerPrinter::ReturnStatus printStatus;
  TransponderReleaseSystem::ReturnStatus trsStatus;
  parameters.insert("personal_account_number",
                    CurrentCommand.value("pan").toString().leftJustified(
                        FULL_PAN_CHAR_LENGTH, QChar('F')));

  // Запрашиваем данные о боксе
  emit getBoxData_signal(&parameters, &boxData, &trsStatus);
  if (trsStatus != TransponderReleaseSystem::Completed) {
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(trsStatus));
    return;
  }

  // Запрашиваем печать бокса
  emit printBoxSticker_signal(&boxData, &printStatus);
  if (printStatus != AbstractStickerPrinter::Completed) {
    CurrentResponse["return_status"] = QString::number(BoxStickerPrintError);
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ProductionLineConnection::processPrintLastBoxSticker() {
  sendLog("Выполнение команды print_last_box_sticker. ");

  AbstractStickerPrinter::ReturnStatus printStatus;

  // Печать последнего стикера для бокса
  emit printLastBoxSticker_signal(&printStatus);
  if (printStatus != AbstractStickerPrinter::Completed) {
    CurrentResponse["return_status"] = QString::number(BoxStickerPrintError);
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ProductionLineConnection::processPrintPalletSticker() {
  sendLog("Выполнение команды print_pallet_sticker. ");

  QHash<QString, QString> parameters;
  QHash<QString, QString> palletData;
  AbstractStickerPrinter::ReturnStatus printStatus;
  TransponderReleaseSystem::ReturnStatus trsStatus;
  parameters.insert("personal_account_number",
                    CurrentCommand.value("pan").toString().leftJustified(
                        FULL_PAN_CHAR_LENGTH, QChar('F')));

  // Запрашиваем данные о паллете
  emit getPalletData_signal(&parameters, &palletData, &trsStatus);
  if (trsStatus != TransponderReleaseSystem::Completed) {
    CurrentResponse["return_status"] =
        QString::number(ServerStatusMatchTable.value(trsStatus));
    return;
  }

  // Запрашиваем печать паллеты
  emit printPalletSticker_signal(&palletData, &printStatus);
  if (printStatus != AbstractStickerPrinter::Completed) {
    CurrentResponse["return_status"] = QString::number(PalletStickerPrintError);
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ProductionLineConnection::processPrintLastPalletSticker() {
  sendLog("Выполнение команды print_last_pallet_sticker. ");

  AbstractStickerPrinter::ReturnStatus printStatus;

  // Печать последнего стикера для бокса
  emit printLastPalletSticker_signal(&printStatus);
  if (printStatus != AbstractStickerPrinter::Completed) {
    CurrentResponse["return_status"] = QString::number(PalletStickerPrintError);
    return;
  }

  CurrentResponse["return_status"] = QString::number(NoError);
}

void ProductionLineConnection::createSocket(qintptr socketDescriptor) {
  Socket = new QTcpSocket(this);
  Socket->setSocketDescriptor(socketDescriptor);

  connect(Socket, &QTcpSocket::readyRead, this,
          &ProductionLineConnection::socketReadyRead_slot);
  connect(Socket, &QTcpSocket::disconnected, this,
          &ProductionLineConnection::socketDisconnected_slot);
  connect(
      Socket,
      QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
      this, &ProductionLineConnection::socketError_slot);
}

void ProductionLineConnection::createExpirationTimer() {
  // Таймер для отсчета времени экспирации
  ExpirationTimer = new QTimer(this);
  ExpirationTimer->setInterval(MaximumConnectionTime);
  // Если время подключения вышло, то вызываем соответствующий обработчик
  connect(ExpirationTimer, &QTimer::timeout, this,
          &ProductionLineConnection::expirationTimerTimeout_slot);
  // Если время подключения вышло, то останавливаем таймер экспирации
  connect(ExpirationTimer, &QTimer::timeout, ExpirationTimer, &QTimer::stop);
  // Если произошла ошибка сети, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::errorOccurred, ExpirationTimer, &QTimer::stop);
  // Если клиент отключился, то останавливаем таймер экспирации
  connect(Socket, &QTcpSocket::disconnected, ExpirationTimer, &QTimer::stop);

  // Запускаем таймер экспирации
  ExpirationTimer->start();
}

void ProductionLineConnection::createDataBlockWaitTimer() {
  // Таймер ожидания для приема блоков данных по частям
  DataBlockWaitTimer = new QTimer(this);
  DataBlockWaitTimer->setInterval(DATA_BLOCK_PART_WAIT_TIME);
  // Если время ожидания вышло, то вызываем соответствующий обработчик
  connect(DataBlockWaitTimer, &QTimer::timeout, this,
          &ProductionLineConnection::dataBlockWaitTimerTimeout_slot);
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

void ProductionLineConnection::createGenerator() {
  Generator = std::make_unique(
      new FirmwareGenerationSystem("FirmwareGenerationSystem"));

  connect(Generator, &FirmwareGenerationSystem::logging, LogSystem::instance(),
          &LogSystem::generate);
}

void ProductionLineConnection::createCommandHandlers() {
  //  connect(newClient, &ProductionLineConnection::authorize_signal, Releaser,
  //          &TransponderReleaseSystem::authorize,
  //          Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::release_signal, Releaser,
  //          &TransponderReleaseSystem::release, Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::confirmRelease_signal,
  //  Releaser,
  //          &TransponderReleaseSystem::confirmRelease,
  //          Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::rerelease_signal, Releaser,
  //          &TransponderReleaseSystem::rerelease,
  //          Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::confirmRerelease_signal,
  //          Releaser, &TransponderReleaseSystem::confirmRerelease,
  //          Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::search_signal, Releaser,
  //          &TransponderReleaseSystem::search, Qt::BlockingQueuedConnection);
  //  connect(newClient,
  //  &ProductionLineConnection::productionLineRollback_signal,
  //          Releaser, &TransponderReleaseSystem::rollbackProductionLine,
  //          Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::getBoxData_signal, Releaser,
  //          &TransponderReleaseSystem::getBoxData,
  //          Qt::BlockingQueuedConnection);
  //  connect(newClient, &ProductionLineConnection::getPalletData_signal,
  //  Releaser,
  //          &TransponderReleaseSystem::getPalletData,
  //          Qt::BlockingQueuedConnection);

  CommandHandlers.insert("echo", &ProductionLineConnection::processEcho);
  CommandHandlers.insert("authorization",
                         &ProductionLineConnection::processAuthorization);
  CommandHandlers.insert("transponder_release",
                         &ProductionLineConnection::processTransponderRelease);
  CommandHandlers.insert(
      "transponder_release_confirm",
      &ProductionLineConnection::processTransponderReleaseConfirm);
  CommandHandlers.insert(
      "transponder_rerelease",
      &ProductionLineConnection::processTransponderRerelease);
  CommandHandlers.insert(
      "transponder_rerelease_confirm",
      &ProductionLineConnection::processTransponderRereleaseConfirm);
  CommandHandlers.insert(
      "production_line_rollback",
      &ProductionLineConnection::processProductionLineRollback);
  CommandHandlers.insert("print_box_sticker",
                         &ProductionLineConnection::processPrintBoxSticker);
  CommandHandlers.insert("print_last_box_sticker",
                         &ProductionLineConnection::processPrintLastBoxSticker);
  CommandHandlers.insert("print_pallet_sticker",
                         &ProductionLineConnection::processPrintPalletSticker);
  CommandHandlers.insert(
      "print_last_pallet_sticker",
      &ProductionLineConnection::processPrintLastPalletSticker);
}

void ProductionLineConnection::createCommandTemplates() {
  QSharedPointer<QVector<QString>> echoSyntax(new QVector<QString>());
  echoSyntax->append("data");
  CommandTemplates.insert("echo", echoSyntax);

  QSharedPointer<QVector<QString>> authorizationSyntax(new QVector<QString>());
  authorizationSyntax->append("login");
  authorizationSyntax->append("password");
  CommandTemplates.insert("authorization", authorizationSyntax);

  QSharedPointer<QVector<QString>> transponderReleaseSyntax(
      new QVector<QString>());
  transponderReleaseSyntax->append("login");
  transponderReleaseSyntax->append("password");
  CommandTemplates.insert("transponder_release", transponderReleaseSyntax);

  QSharedPointer<QVector<QString>> transponderReleaseConfirmSyntax(
      new QVector<QString>());
  transponderReleaseConfirmSyntax->append("login");
  transponderReleaseConfirmSyntax->append("password");
  transponderReleaseConfirmSyntax->append("ucid");
  CommandTemplates.insert("transponder_release_confirm",
                          transponderReleaseConfirmSyntax);

  QSharedPointer<QVector<QString>> transponderRereleaseSyntax(
      new QVector<QString>());
  transponderRereleaseSyntax->append("pan");
  CommandTemplates.insert("transponder_rerelease", transponderRereleaseSyntax);

  QSharedPointer<QVector<QString>> transponderRereleaseConfirmSyntax(
      new QVector<QString>());
  transponderRereleaseConfirmSyntax->append("pan");
  transponderRereleaseConfirmSyntax->append("ucid");
  CommandTemplates.insert("transponder_rerelease_confirm",
                          transponderRereleaseConfirmSyntax);

  QSharedPointer<QVector<QString>> productionLineRollbackSyntax(
      new QVector<QString>());
  productionLineRollbackSyntax->append("login");
  productionLineRollbackSyntax->append("password");
  CommandTemplates.insert("production_line_rollback",
                          productionLineRollbackSyntax);

  QSharedPointer<QVector<QString>> printBoxStickerSyntax(
      new QVector<QString>());
  printBoxStickerSyntax->append("pan");
  CommandTemplates.insert("print_box_sticker", printBoxStickerSyntax);

  QSharedPointer<QVector<QString>> printLastBoxStickerSyntax(
      new QVector<QString>());
  CommandTemplates.insert("print_last_box_sticker", printLastBoxStickerSyntax);

  QSharedPointer<QVector<QString>> printPalletStickerSyntax(
      new QVector<QString>());
  printPalletStickerSyntax->append("pan");
  CommandTemplates.insert("print_pallet_sticker", printPalletStickerSyntax);

  QSharedPointer<QVector<QString>> printLastPalletStickerSyntax(
      new QVector<QString>());
  CommandTemplates.insert("print_last_pallet_sticker",
                          printLastPalletStickerSyntax);
}

void ProductionLineConnection::createServerStatusMatchTable() {
  ServerStatusMatchTable.insert(TransponderReleaseSystem::DatabaseQueryError,
                                DatabaseError);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::DatabaseTransactionError, DatabaseError);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::DatabaseConnectionError, DatabaseError);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::TransponderNotFound,
                                TransponderNotFound);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::TransponderNotReleasedEarlier,
      TransponderNotReleasedEarlier);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::TransponderNotReleasedEarlier,
      TransponderNotReleasedEarlier);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::FreeBoxMissed,
                                FreeBoxMissed);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::AwaitingConfirmationError,
      AwaitingConfirmationError);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::IdenticalUcidError,
                                IdenticalUcidError);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::ProductionLineMissed,
                                ProductionLineMissed);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::ProductionLineNotActive,
      ProductionLineNotActive);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::CurrentOrderRunOut,
                                CurrentOrderRunOut);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::CurrentOrderAssembled,
                                CurrentOrderAssembled);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::ProductionLineRollbackLimitError,
      ProductionLineRollbackLimitError);
  ServerStatusMatchTable.insert(TransponderReleaseSystem::BoxStickerPrintError,
                                BoxStickerPrintError);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::PalletStickerPrintError,
      PalletStickerPrintError);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::NextTransponderNotFound,
      NextTransponderNotFound);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::StartBoxAssemblingError,
      StartBoxAssemblingError);
  ServerStatusMatchTable.insert(
      TransponderReleaseSystem::StartPalletAssemblingError,
      StartPalletAssemblingError);
}

void ProductionLineConnection::socketReadyRead_slot() {
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

void ProductionLineConnection::socketDisconnected_slot() {
  sendLog("Отключился. ");

  // Отправляем сигнал об отключении клиента
  emit disconnected();
}

void ProductionLineConnection::socketError_slot(
    QAbstractSocket::SocketError socketError) {
  // Если клиент отключился не самостоятельно
  if (socketError != 1) {
    sendLog(QString("Ошибка сети: %1. %2.")
                .arg(QString::number(socketError), Socket->errorString()));
    Socket->close();
  }
}

void ProductionLineConnection::expirationTimerTimeout_slot() {
  sendLog("Экспирация времени подключения. ");

  // Закрываем соединение
  Socket->close();
}

void ProductionLineConnection::dataBlockWaitTimerTimeout_slot() {
  sendLog("Время ожидания вышло. Блок данных сбрасывается. ");
  ReceivedDataBlock.clear();
  ReceivedDataBlockSize = 0;
}

ProductionLineConnection::ProductionLineConnection() {}
