#include "server_manager.h"

/* Диспетчер DSRC  */
//==================================================================================
ServerManager::ServerManager(QObject* parent) : QObject(parent) {
  setObjectName("ServerManager");

  // Буфер для отображения данных базы данных
  Buffer = new DatabaseBuffer(this);

  // Создаем среду выполнения для хоста
  createHostInstance();

  // Создаем среду выполнения для инициализатора
  createInitializerInstance();

  // Создаем таймеры
  createTimers();

  // Создаем цикл ожидания
  createWaitingLoop();

  // Готовы к выполнению операций
  CurrentState = Ready;
}

ServerManager::~ServerManager() {
  ServerThread->quit();
  ServerThread->wait();

  InitializerThread->quit();
  InitializerThread->wait();
}

DatabaseBuffer* ServerManager::buffer() {
  return Buffer;
}

void ServerManager::applySettings() {
  emit logging("Применение новых настроек. ");

  // Посылаем общий сигнал для применения настроек
  emit applySettings_signal();
}

void ServerManager::start() {
  if ((Host) && (Host->isListening())) {
    emit logging("Сервер уже запущен. ");
    return;
  }

  emit logging("Запуск сервера персонализации. ");
  emit serverStart_signal();
}

void ServerManager::stop() {
  emit logging("Остановка сервера персонализации. ");
  emit serverStop_signal();
}

void ServerManager::showDatabaseTable(const QString& name) {
  if (!startOperationExecution("showDatabaseTable")) {
    return;
  }

  Buffer->clear();

  emit logging("Представление линий производства. ");
  emit getDatabaseTable_signal(name, Buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  endOperationExecution("showDatabaseTable");
}

void ServerManager::showCustomResponse(const QString& req) {
  Buffer->clear();

  emit logging("Представление ответа на кастомный запрос. ");
  emit getCustomResponse_signal(req, Buffer);
}

void ServerManager::createHostInstance() {
  // Создаем сервер и поток для него
  Host = new PersoHost(nullptr);
  ServerThread = new QThread(this);

  // Переносим сервер в поток
  Host->moveToThread(ServerThread);

  // Подключаем логгирование к серверу
  connect(Host, &PersoHost::logging, this, &ServerManager::proxyLogging);
  // Подключаем сигнал для применения новых настроек
  connect(this, &ServerManager::applySettings_signal, Host,
          &PersoHost::applySettings);
  // Когда поток завершит работу, Server будет удален
  connect(ServerThread, &QThread::finished, Host, &QObject::deleteLater);
  // Когда поток завершит работу, он будет удален
  connect(ServerThread, &QThread::finished, ServerThread,
          &QObject::deleteLater);
  // Когда поток завершит работу, вызываем метод обработки
  connect(ServerThread, &QThread::finished, this,
          &ServerManager::on_ServerThreadFinished_slot);

  // Подключаем функционал
  connect(this, &ServerManager::serverStart_signal, Host, &PersoHost::start);
  connect(this, &ServerManager::serverStop_signal, Host, &PersoHost::stop);

  // Запускаем поток сервера
  ServerThread->start();
}

void ServerManager::createInitializerInstance() {
  // Создаем инициализатор и поток с базой данных для него
  Initializer = new TransponderInitializer(nullptr);
  InitializerThread = new QThread(this);

  // Переносим инициализатор в поток
  Initializer->moveToThread(InitializerThread);

  // Подключаем логгирование к инициализатору
  connect(Initializer, &TransponderInitializer::logging, this,
          &ServerManager::proxyLogging);
  // Подключаем сигнал для применения новых настроек
  connect(this, &ServerManager::applySettings_signal, Initializer,
          &TransponderInitializer::applySettings);
  // Когда поток завершит работу, инициализатор будет удален
  connect(InitializerThread, &QThread::finished, Initializer,
          &QObject::deleteLater);
  // Когда поток завершит работу, он будет удален
  connect(InitializerThread, &QThread::finished, InitializerThread,
          &QObject::deleteLater);
  // Когда поток завершит работу, вызываем метод обработки
  connect(InitializerThread, &QThread::finished, this,
          &ServerManager::on_InitializerThreadFinished_slot);
  // Создаем контроллер базы данных после запуска потока
  connect(InitializerThread, &QThread::started, Initializer,
          &TransponderInitializer::createDatabaseController);

  // Подключаем функционал
  connect(this, &ServerManager::getDatabaseTable_signal, Initializer,
          &TransponderInitializer::getDatabaseTable);
  connect(this, &ServerManager::getCustomResponse_signal, Initializer,
          &TransponderInitializer::getCustomResponse);
  connect(Initializer, &TransponderInitializer::operationFinished, this,
          &ServerManager::on_InitializerFinished_slot);

  // Запускаем поток инициализатора
  InitializerThread->start();
}

void ServerManager::createWaitingLoop() {
  WaitingLoop = new QEventLoop(this);
  connect(this, &ServerManager::waitingEnd, WaitingLoop, &QEventLoop::quit);
}

void ServerManager::createTimers() {
  // Таймер, отслеживающий длительность выполняющихся операций
  ODTimer = new QTimer(this);
  ODTimer->setInterval(SERVER_MANAGER_OPERATION_MAX_DURATION);
  connect(ODTimer, &QTimer::timeout, this,
          &ServerManager::on_ODTimerTimeout_slot);
  connect(ODTimer, &QTimer::timeout, ODTimer, &QTimer::stop);
  connect(this, &ServerManager::operationPerformingEnded, ODTimer,
          &QTimer::stop);

  // Таймер для измерения длительности операции
  ODMeter = new QElapsedTimer();
}

void ServerManager::setupODQTimer(uint32_t msecs) {
  // Таймер, отслеживающий квант длительности операции
  ODQTimer = new QTimer(this);
  ODQTimer->setInterval(msecs);

  connect(ODQTimer, &QTimer::timeout, this,
          &ServerManager::on_ODQTimerTimeout_slot);
  connect(this, &ServerManager::operationPerformingEnded, ODQTimer,
          &QTimer::stop);
}

bool ServerManager::startOperationExecution(const QString& operationName) {
  // Проверяем готовность к выполнению операции
  if (CurrentState != Ready)
    return false;

  // Переходим в состояние ожидания конца обработки
  CurrentState = WaitingExecution;

  //  // Настраиваем и запускаем таймер для измерения квантов времени
  QSettings settings;
  uint64_t operationDuration = settings
                                   .value(QString("ServerManager/Operations/") +
                                          operationName + QString("/Duration"))
                                   .toInt();
  uint32_t operationQuantDuration = operationDuration / 100;
  operationQuantDuration++;
  emit logging(QString("Длительность кванта операции: %1.")
                   .arg(QString::number(operationQuantDuration)));
  setupODQTimer(operationQuantDuration);
  ODQTimer->start();

  //  // Запускаем таймер для контроля максимальной длительности операции
  ODTimer->start();

  //  // Запускаем измеритель длительности операции
  ODMeter->start();

  // Отправляем сигнал о начале выполнения длительной операции
  emit operationPerfomingStarted();

  return true;
}

void ServerManager::endOperationExecution(const QString& operationName) {
  QSettings settings;

  // Измеряем и сохраняем длительность операции
  uint64_t duration = ODMeter->elapsed();
  emit logging(
      QString("Длительность операции: %1.").arg(QString::number(duration)));
  settings.setValue(QString("ServerManager/Operations/") + operationName +
                        QString("/Duration"),
                    duration);

  // Сигнал о завершении текущей операции
  emit operationPerformingEnded();

  // Оповещаем пользователя о результатах
  if (CurrentState == Completed) {
    emit notifyUser(NotificarionText);
  } else {
    emit notifyUserAboutError(NotificarionText);
  }

  // Готовы к следующей операции
  CurrentState = Ready;
}

/*
 * Приватные слоты
 */

void ServerManager::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("PersoHost"))
    emit logging(QString("Host - ") + log);
  else if (sender()->objectName() == QString("TransponderInitializer"))
    emit logging(QString("Initializer - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void ServerManager::on_ServerThreadFinished_slot() {
  emit logging("Поток сервера завершился. ");
  Host = nullptr;
  ServerThread = nullptr;
}

void ServerManager::on_InitializerThreadFinished_slot() {
  emit logging("Поток инициализатора завершился. ");
  Initializer = nullptr;
  InitializerThread = nullptr;
}

void ServerManager::on_InitializerFinished_slot(
    TransponderInitializer::ExecutionStatus status) {
  switch (status) {
    case TransponderInitializer::NotExecuted:
      CurrentState = Failed;
      NotificarionText = "Инициализатор: операция не была запущена. ";
      emit break;
    case TransponderInitializer::DatabaseConnectionError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: не удалось подключиться к базе данных. ";
      break;
    case TransponderInitializer::DatabaseQueryError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: ошибка при выполнении запроса к базе данных. ";
      break;
    case TransponderInitializer::UnknowError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: получена неизвестная ошибка при выполнении "
          "операции. ";
      break;
    case TransponderInitializer::CompletedSuccessfully:
      CurrentState = Completed;
      NotificarionText = "Операция успешно выполнена. ";
      break;
  }

  // Завершаем цикл ожидания
  emit waitingEnd();
}

void ServerManager::on_ODTimerTimeout_slot() {
  emit logging("Операция выполняется слишком долго. Сброс. ");
  emit notifyUserAboutError("Операция выполняется слишком долго. Сброс. ");
}

void ServerManager::on_ODQTimerTimeout_slot() {
  emit operationStepPerfomed();
}

//==================================================================================
