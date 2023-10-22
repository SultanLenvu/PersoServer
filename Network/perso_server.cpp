#include "perso_server.h"

PersoServer::PersoServer(QObject* parent) : QTcpServer(parent) {
  setObjectName("PersoServer");
  CurrentState = Idle;
  MaxNumberClientConnections = 0;

  // Загружаем настройки
  loadSettings();

  // Создаем идентификаторы для клиентов
  createClientIdentifiers();

  // Создаем систему выпуска транспондеров
  createReleaserInstance();

  // Создаем принтеры
  createStickerPrinters();

  // Создаем таймер перезапуска
  createRestartTimer();
}

PersoServer::~PersoServer() {
  if (ReleaserThread->isRunning()) {
    ReleaserThread->quit();
    ReleaserThread->wait();
  }

  for (QHash<int32_t, QThread*>::iterator it = ClientThreads.begin();
       it != ClientThreads.end(); it++) {
    (*it)->exit();
    (*it)->wait();
  }
}

bool PersoServer::start() {
  sendLog("Проверка конфигурации");
  if (!checkConfiguration()) {
    sendLog("Проверка конфигурации провалена. Запуск сервера невозможен.");
    RestartTimer->start();
    return false;
  }

  // Запускаем систему выпуска транспондеров
  TransponderReleaseSystem::ReturnStatus status;
  emit startReleaser_signal(&status);
  if (status != TransponderReleaseSystem::Completed) {
    sendLog(
        "Не удалось запустить систему выпуска транспондеров. Запуск сервера "
        "невозможен.");
    RestartTimer->start();
    return false;
  }

  // Поднимаем сервер
  sendLog(
      QString("Попытка запуска на %1:%2.")
          .arg(ListeningAddress.toString(), QString::number(ListeningPort)));
  if (!listen(ListeningAddress, ListeningPort)) {
    sendLog("Не удалось запуститься. ");
    RestartTimer->start();
    return false;
  }
  // Если сервер поднялся
  if (thread() == QCoreApplication::instance()->thread()) {
    sendLog("Запущен в главном потоке. ");
  } else {
    sendLog("Запущен в отдельном потоке. ");
  }

  // Изменяем состояние
  CurrentState = Work;
  return true;
}

void PersoServer::stop() {
  // Останавливаем релизер
  Releaser->stop();

  // Останавливаем сервер
  close();
  sendLog("Остановлен. ");

  // Останавливаем систему выпуска транспондеров
  emit stopReleaser_signal();

  CurrentState = Idle;
}

void PersoServer::incomingConnection(qintptr socketDescriptor) {
  sendLog("Получен запрос на новое подключение. ");

  if (CurrentState == Panic) {
    sendLog(
        "В процессе функционирования получена критическая ошибка. Обработка "
        "новых клиентских подключений невозможна.");
    return;
  }

  // Если свободных идентификаторов нет
  if (FreeClientIds.isEmpty()) {
    pauseAccepting();  // Приостанавливаем прием новых подключений
    CurrentState = Paused;

    sendLog("Достигнут лимит подключений, прием новых приостановлен. ");
    return;
  }

  // Создаем среду выполнения для клиента
  createClientInstance(socketDescriptor);

  // Проверяем созданную среду выполнения
  emit checkNewClientInstance();
}

void PersoServer::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();

  RestartPeriod = settings.value("perso_server/restart_period").toInt();
  MaxNumberClientConnections =
      settings.value("perso_server/max_number_client_connection").toInt();

  ListeningAddress = settings.value("perso_server/listen_ip").toString();
  ListeningPort = settings.value("perso_server/listen_port").toInt();

  PrinterForBoxSticker =
      settings.value("perso_server/printer_for_box_sticker").toString();
  PrinterForPalletSticker =
      settings.value("perso_server/printer_for_pallet_sticker").toString();
}

void PersoServer::sendLog(const QString& log) const {
  if (LogEnable) {
    emit logging("PersoServer - " + log);
  }
}

void PersoServer::processCriticalError(const QString& log) {
  QString msg("Паника. Получена критическая ошибка. ");
  sendLog(msg + log);
  CurrentState = Panic;
}

bool PersoServer::checkConfiguration() {
  sendLog("Проверка конфигурации. ");

  if (!BoxStickerPrinter->checkConfiguration()) {
    sendLog(
        "Проверка конфигурации принтера для печати стикеров на боксы "
        "провалена. ");
    return false;
  }

  if (!PalletStickerPrinter->checkConfiguration()) {
    sendLog(
        "Проверка конфигурации принтера для печати стикеров на паллеты "
        "провалена. ");
    return false;
  }

  sendLog("Проверка конфигурации прошла успешно. ");
  return true;
}

void PersoServer::createReleaserInstance() {
  Releaser = new TransponderReleaseSystem(nullptr);
  connect(this, &PersoServer::startReleaser_signal, Releaser,
          &TransponderReleaseSystem::start, Qt::BlockingQueuedConnection);
  connect(this, &PersoServer::stopReleaser_signal, Releaser,
          &TransponderReleaseSystem::stop, Qt::BlockingQueuedConnection);
  connect(Releaser, &TransponderReleaseSystem::logging, LogSystem::instance(),
          &LogSystem::generate);
  connect(Releaser, &TransponderReleaseSystem::boxAssemblingFinished, this,
          &PersoServer::printBoxSticker_slot);
  connect(Releaser, &TransponderReleaseSystem::palletAssemblingFinished, this,
          &PersoServer::printPalletSticker_slot);
  connect(Releaser, &TransponderReleaseSystem::failed, this,
          &PersoServer::on_ReleaserFailed_slot);

  // Создаем отдельный поток для системы выпуска транспондеров
  ReleaserThread = new QThread(this);
  Releaser->moveToThread(ReleaserThread);

  connect(ReleaserThread, &QThread::finished, ReleaserThread,
          &QThread::deleteLater);
  connect(ReleaserThread, &QThread::finished, Releaser,
          &PersoClient::deleteLater);
  connect(ReleaserThread, &QThread::started, Releaser,
          &TransponderReleaseSystem::on_InstanceThreadStarted_slot);

  // Запускаем поток
  ReleaserThread->start();
}

void PersoServer::createClientIdentifiers() {
  FreeClientIds.clear();
  for (int32_t i = 1; i <= MaxNumberClientConnections; i++) {
    FreeClientIds.push(i);
  }
}

void PersoServer::createClientInstance(qintptr socketDescriptor) {
  // Выделяем свободный идентификатор
  int32_t clientId = FreeClientIds.pop();

  // Создаем новое клиент-подключение
  PersoClient* newClient = new PersoClient(clientId, socketDescriptor);

  connect(newClient, &PersoClient::logging, LogSystem::instance(),
          &LogSystem::generate);
  connect(newClient, &PersoClient::disconnected, this,
          &PersoServer::on_ClientDisconnected_slot);
  connect(this, &PersoServer::checkNewClientInstance, newClient,
          &PersoClient::instanceTesting);

  // Добавляем клиента в реестр
  Clients.insert(clientId, newClient);
  sendLog(QString("Новый клиент создан и зарегистрирован в реестре с "
                  "идентификатором %1. ")
              .arg(QString::number(newClient->getId())));

  // Создаем отдельный поток для клиента
  QThread* newClientThread = new QThread(this);
  newClient->moveToThread(newClientThread);

  connect(newClient, &PersoClient::disconnected, newClientThread,
          &QThread::quit);
  connect(newClientThread, &QThread::finished, newClientThread,
          &QThread::deleteLater);
  connect(newClientThread, &QThread::finished, newClient,
          &PersoClient::deleteLater);
  connect(newClientThread, &QThread::destroyed, this,
          &PersoServer::on_ClientThreadDeleted_slot);

  // Добавляем поток в соответствующий реестр
  ClientThreads.insert(clientId, newClientThread);

  // Соединяем клиента с системой выпуска транспондеров
  connect(newClient, &PersoClient::releaserAuthorize_signal, Releaser,
          &TransponderReleaseSystem::authorize);
  connect(newClient, &PersoClient::releaseRelease_signal, Releaser,
          &TransponderReleaseSystem::release);
  connect(newClient, &PersoClient::releaserConfirmRelease_signal, Releaser,
          &TransponderReleaseSystem::confirmRelease);
  connect(newClient, &PersoClient::releaserRerelease_signal, Releaser,
          &TransponderReleaseSystem::rerelease);
  connect(newClient, &PersoClient::releaserConfirmRerelease_signal, Releaser,
          &TransponderReleaseSystem::confirmRerelease);
  connect(newClient, &PersoClient::releaserSearch_signal, Releaser,
          &TransponderReleaseSystem::search);

  // Подключаем принтер
  connect(newClient, &PersoClient::printBoxSticker_signal, this,
          &PersoServer::printBoxSticker_slot);
  connect(newClient, &PersoClient::printLastBoxSticker_signal, this,
          &PersoServer::printLastBoxSticker_slot);
  connect(newClient, &PersoClient::printPalletSticker_signal, this,
          &PersoServer::printPalletSticker_slot);
  connect(newClient, &PersoClient::printLastPalletSticker_signal, this,
          &PersoServer::printLastPalletSticker_slot);

  // Запускаем поток
  newClientThread->start();
  sendLog("Клиентский поток запущен. ");
}

void PersoServer::createStickerPrinters() {
  BoxStickerPrinter = new TE310Printer(this, PrinterForBoxSticker);
  connect(BoxStickerPrinter, &IStickerPrinter::logging, LogSystem::instance(),
          &LogSystem::generate);

  PalletStickerPrinter = new TE310Printer(this, PrinterForPalletSticker);
  connect(PalletStickerPrinter, &IStickerPrinter::logging,
          LogSystem::instance(), &LogSystem::generate);
}

void PersoServer::createRestartTimer() {
  RestartTimer = new QTimer(this);
  RestartTimer->setInterval(RestartPeriod * 1000);
  connect(RestartTimer, &QTimer::timeout, this,
          &PersoServer::on_RestartTimerTimeout_slot);
}

void PersoServer::on_ClientDisconnected_slot() {
  PersoClient* disconnectedClient = dynamic_cast<PersoClient*>(sender());
  if (!disconnectedClient) {
    processCriticalError(
        "Не удалось получить доступ к данным отключившегося клиента. ");
    return;
  }
  // Освобождаем занятый идентификатор
  uint32_t clientId = disconnectedClient->getId();
  FreeClientIds.push(clientId);

  // Удаляем отключившегося клиента и его поток из соответствующих реестров
  disconnectedClient->thread()->quit();
  if (!disconnectedClient->thread()->wait()) {
    processCriticalError(
        "Не удалось остановить поток отключившегося клиента. ");
  } else {
    sendLog(QString("Поток клиента %1 остановлен. ")
                .arg(QString::number(clientId)));
  }
  ClientThreads.remove(clientId);
  Clients.remove(clientId);

  sendLog(
      QString("Клиент %1 удален из реестра. ").arg(QString::number(clientId)));

  // Если ранее был достигнут лимит подключений
  if (CurrentState == Paused) {
    resumeAccepting();  // Продолжаем прием запросов на подключение
    CurrentState = Work;
  }
}

void PersoServer::on_ClientThreadDeleted_slot() {
  sendLog(QString("Клиентский поток удален. "));
}

void PersoServer::printBoxSticker_slot(
    const QSharedPointer<QHash<QString, QString> > data) {
  sendLog("Запуск печати стикера для бокса.");

  IStickerPrinter::ReturnStatus status =
      BoxStickerPrinter->printBoxSticker(data.get());

  if (status != IStickerPrinter::Completed) {
    processCriticalError("Получена ошибка при печати стикера для бокса. ");
  }
}

void PersoServer::printLastBoxSticker_slot() {
  sendLog("Запуск печати стикера для бокса.");

  IStickerPrinter::ReturnStatus status =
      BoxStickerPrinter->printLastBoxSticker();

  if (status != IStickerPrinter::Completed) {
    processCriticalError("Получена ошибка при печати стикера для бокса. ");
  }
}

void PersoServer::printPalletSticker_slot(
    const QSharedPointer<QHash<QString, QString> > data) {
  sendLog("Запуск печати стикера для паллеты.");

  IStickerPrinter::ReturnStatus status =
      PalletStickerPrinter->printPalletSticker(data.get());

  if (status != IStickerPrinter::Completed) {
    processCriticalError("Получена ошибка при печати стикера для паллеты. ");
  }
}

void PersoServer::printLastPalletSticker_slot() {
  sendLog("Запуск печати стикера для паллеты.");

  IStickerPrinter::ReturnStatus status =
      PalletStickerPrinter->printLastPalletSticker();

  if (status != IStickerPrinter::Completed) {
    processCriticalError("Получена ошибка при печати стикера для паллеты. ");
  }
}

void PersoServer::on_RestartTimerTimeout_slot() {
  if (start()) {
    RestartTimer->stop();
  }
}

void PersoServer::on_ReleaserFailed_slot(
    TransponderReleaseSystem::ReturnStatus status) {
  processCriticalError("Система выпуска транспондеров неисправна.");
}