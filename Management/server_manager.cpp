#include "server_manager.h"

/* Диспетчер DSRC  */
//==================================================================================
ServerManager::ServerManager(QObject* parent) : QObject(parent) {
  setObjectName("ServerManager");

  // Создаем модель для представления таблицы базы данных
  Buffer = new DatabaseTableModel(this);

  // Создаем среду выполнения для хоста
  createHostInstance();

  // Создаем среду выполнения для инициализатора
  createOrderCreatorInstance();

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

  OrderCreatorThread->quit();
  OrderCreatorThread->wait();
}

DatabaseTableModel* ServerManager::buffer(void) {
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
  // Начинаем выполнение операции
  if (!startOperationExecution("showDatabaseTable")) {
    return;
  }

  Buffer->clear();

  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal(name, Buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showDatabaseTable");
}

void ServerManager::clearDatabaseTable(const QString& name) {
  // Начинаем выполнение операции
  if (!startOperationExecution("clearDatabaseTable")) {
    return;
  }

  emit logging("Очистка таблицы базы данных. ");
  emit clearDatabaseTable_signal(name);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  if (CurrentState != Completed) {
    // Завершаем выполнение операции
    endOperationExecution("clearDatabaseTable");
    return;
  }

  Buffer->clear();
  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal(name, Buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("clearDatabaseTable");
}

void ServerManager::showCustomResponse(const QString& req) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showCustomResponse")) {
    return;
  }

  emit logging("Представление ответа на кастомный запрос. ");
  emit getCustomResponse_signal(req, Buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showCustomResponse");
}

void ServerManager::createNewOrder(IssuerOrder* newOrder) {
  if (!startOperationExecution("createNewOrder")) {
    return;
  }

  emit logging("Создание нового заказа. ");
  emit createNewOrder_signal(newOrder);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewOrder");
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

void ServerManager::createOrderCreatorInstance() {
  // Создаем строитель и поток для создателя отчетов
  OrderCreatorBuilder = new OCSBuilder();
  OrderCreatorThread = new QThread(this);

  // Переносим инициализатор в поток
  OrderCreatorBuilder->moveToThread(OrderCreatorThread);

  // Когда поток завершит работу, он будет удален
  connect(OrderCreatorThread, &QThread::finished, OrderCreatorThread,
          &QObject::deleteLater);
  // Когда поток завершит работу, вызываем метод обработки
  connect(OrderCreatorThread, &QThread::finished, this,
          &ServerManager::on_OrderCreatorThreadFinished_slot);
  // Когда поток завершит работу, строитель будет удален
  connect(OrderCreatorThread, &QThread::finished, OrderCreatorBuilder,
          &QObject::deleteLater);
  // Когда поток начнет свою работу, строитель создаст в нем составитель отчетов
  connect(OrderCreatorThread, &QThread::started, OrderCreatorBuilder,
          &OCSBuilder::build);

  // Когда строитель завершит работу, возвращем его в менеджер
  connect(OrderCreatorBuilder, &OCSBuilder::completed, this,
          &ServerManager::on_OrderCreatorBuilderCompleted_slot);

  // Запускаем поток инициализатора
  OrderCreatorThread->start();
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

  // Очищаем буфер
  Buffer->clear();

  // Переходим в состояние ожидания конца обработки
  CurrentState = WaitingExecution;

  //  Настраиваем и запускаем таймер для измерения квантов времени
  QSettings settings;
  uint64_t operationDuration = settings
                                   .value(QString("ServerManager/Operations/") +
                                          operationName + QString("/Duration"))
                                   .toInt();
  uint32_t operationQuantDuration = operationDuration / 100;
  operationQuantDuration += 10;
  emit logging(QString("Длительность кванта операции: %1.")
                   .arg(QString::number(operationQuantDuration)));
  setupODQTimer(operationQuantDuration);
  ODQTimer->start();

  // Запускаем таймер для контроля максимальной длительности операции
  ODTimer->start();

  // Запускаем измеритель длительности операции
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
  else if (sender()->objectName() == QString("OrderSystem"))
    emit logging(QString("OrderCreator - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void ServerManager::on_OrderCreatorBuilderCompleted_slot() {
  OrderCreator = OrderCreatorBuilder->buildedObject();

  // Когда поток завершит работу, составитель заказов будет удален
  connect(OrderCreatorThread, &QThread::finished, OrderCreator,
          &QObject::deleteLater);
  // Подключаем логгирование к инициализатору
  connect(OrderCreator, &OrderSystem::logging, this,
          &ServerManager::proxyLogging);
  // Подключаем сигнал для применения новых настроек
  connect(this, &ServerManager::applySettings_signal, OrderCreator,
          &OrderSystem::applySettings);
  // После выполнения операции формирователем заказов, оповещаем менеджер
  connect(OrderCreator, &OrderSystem::operationFinished, this,
          &ServerManager::on_OrderCreatorFinished_slot);

  // Подключаем функционал
  connect(this, &ServerManager::getDatabaseTable_signal, OrderCreator,
          &OrderSystem::getDatabaseTable);
  connect(this, &ServerManager::getCustomResponse_signal, OrderCreator,
          &OrderSystem::getCustomResponse);
  connect(this, &ServerManager::createNewOrder_signal, OrderCreator,
          &OrderSystem::createNewOrder);
  connect(this, &ServerManager::clearDatabaseTable_signal, OrderCreator,
          &OrderSystem::clearDatabaseTable);
}

void ServerManager::on_ServerThreadFinished_slot() {
  emit logging("Поток сервера завершился. ");
  Host = nullptr;
  ServerThread = nullptr;
}

void ServerManager::on_OrderCreatorThreadFinished_slot() {
  emit logging("Поток инициализатора завершился. ");
  OrderCreator = nullptr;
  OrderCreatorThread = nullptr;
}

void ServerManager::on_OrderCreatorFinished_slot(
    OrderSystem::ExecutionStatus status) {
  switch (status) {
    case OrderSystem::NotExecuted:
      CurrentState = Failed;
      NotificarionText = "Инициализатор: операция не была запущена. ";
      emit break;
    case OrderSystem::DatabaseConnectionError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: не удалось подключиться к базе данных. ";
      break;
    case OrderSystem::DatabaseQueryError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: ошибка при выполнении запроса к базе данных. ";
      break;
    case OrderSystem::UnknowError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: получена неизвестная ошибка при выполнении "
          "операции. ";
      break;
    case OrderSystem::CompletedSuccessfully:
      CurrentState = Completed;
      NotificarionText = "Операция успешно выполнена. ";
      break;
  }

  // Выходим из цикла ожидания
  emit waitingEnd();
}

void ServerManager::on_ODTimerTimeout_slot() {
  emit logging("Операция выполняется слишком долго. Сброс. ");
  emit notifyUserAboutError("Операция выполняется слишком долго. Сброс. ");

  // Выходим из цикла ожидания
  emit waitingEnd();
}

void ServerManager::on_ODQTimerTimeout_slot() {
  emit operationStepPerfomed();
}

//==================================================================================
