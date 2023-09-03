#include "server_manager.h"

/* Диспетчер DSRC  */
//==================================================================================
ServerManager::ServerManager(QObject* parent) : QObject(parent) {
  setObjectName("ServerManager");

  // Создаем среду выполнения для хоста
  createHostInstance();

  // Создаем среду выполнения для инициализатора
  createAdministratorInstance();

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

  AdministratorThread->quit();
  AdministratorThread->wait();
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

void ServerManager::showDatabaseTable(const QString& name,
                                      DatabaseTableModel* buffer) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showDatabaseTable")) {
    return;
  }

  buffer->clear();
  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal(name, buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showDatabaseTable");
}

void ServerManager::clearDatabaseTable(const QString& name,
                                       DatabaseTableModel* buffer) {
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

  buffer->clear();
  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal(name, buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("clearDatabaseTable");
}

void ServerManager::initIssuers(DatabaseTableModel* buffer) {
  if (!startOperationExecution("initIssuers")) {
    return;
  }

  emit logging("Создание эмитентов. ");
  emit initIssuerTable_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  if (CurrentState != Completed) {
    // Завершаем выполнение операции
    endOperationExecution("clearDatabaseTable");
    return;
  }

  buffer->clear();
  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal("issuers", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("initIssuers");
}

void ServerManager::performCustomRequest(const QString& req,
                                         DatabaseTableModel* buffer) {
  // Начинаем выполнение операции
  if (!startOperationExecution("performCustomRequest")) {
    return;
  }

  buffer->clear();
  emit logging("Представление ответа на кастомный запрос. ");
  emit getCustomResponse_signal(req, buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("performCustomRequest");
}

void ServerManager::createNewOrder(
    const QMap<QString, QString>* orderParameters,
    DatabaseTableModel* buffer) {
  if (!startOperationExecution("createNewOrder")) {
    return;
  }

  emit logging("Создание нового заказа. ");
  emit createNewOrder_signal(orderParameters);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("createNewOrder");
    return;
  }

  buffer->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewOrder");
}

void ServerManager::deleteLastOrder(DatabaseTableModel* buffer) {
  if (!startOperationExecution("createNewOrder")) {
    return;
  }

  emit logging("Удаление последнего заказа. ");
  emit deleteLastOrder_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("createNewOrder");
    return;
  }

  buffer->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewOrder");
}

void ServerManager::showOrderTable(DatabaseTableModel* buffer) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showOrderTable")) {
    return;
  }

  buffer->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showOrderTable");
}

void ServerManager::createNewProductionLine(
    const QMap<QString, QString>* productionLineParameters,
    DatabaseTableModel* buffer) {
  if (!startOperationExecution("createNewProductionLine")) {
    return;
  }

  emit logging("Создание новой линии производства. ");
  emit createNewProductionLine_signal(productionLineParameters);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("createNewProductionLine");
    return;
  }

  buffer->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewProductionLine");
}

void ServerManager::deleteLastProductionLine(DatabaseTableModel* buffer) {
  if (!startOperationExecution("deleteLastProductionLine")) {
    return;
  }

  emit logging("Удаление последней линии производства. ");
  emit deleteLastProductionLines_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("deleteLastProductionLine");
    return;
  }

  buffer->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("deleteLastProductionLine");
}

void ServerManager::showProductionLineTable(DatabaseTableModel* buffer) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showProductionLineTable")) {
    return;
  }

  buffer->clear();
  emit logging("Отображение линий производства. ");
  emit getDatabaseTable_signal("production_lines", buffer);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showProductionLineTable");
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

void ServerManager::createAdministratorInstance() {
  // Создаем строитель и поток для создателя отчетов
  AdministratorBuilder = new AdministrationSystemBuilder();
  AdministratorThread = new QThread(this);

  // Переносим инициализатор в поток
  AdministratorBuilder->moveToThread(AdministratorThread);

  // Когда поток завершит работу, он будет удален
  connect(AdministratorThread, &QThread::finished, AdministratorThread,
          &QObject::deleteLater);
  // Когда поток завершит работу, вызываем метод обработки
  connect(AdministratorThread, &QThread::finished, this,
          &ServerManager::on_AdministratorThreadFinished_slot);
  // Когда поток завершит работу, строитель будет удален
  connect(AdministratorThread, &QThread::finished, AdministratorBuilder,
          &QObject::deleteLater);
  // Когда поток начнет свою работу, строитель создаст в нем составитель отчетов
  connect(AdministratorThread, &QThread::started, AdministratorBuilder,
          &AdministrationSystemBuilder::build);

  // Когда строитель завершит работу, возвращем его в менеджер
  connect(AdministratorBuilder, &AdministrationSystemBuilder::completed, this,
          &ServerManager::on_AdministratorBuilderCompleted_slot);

  // Запускаем поток инициализатора
  AdministratorThread->start();
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
  else if (sender()->objectName() == QString("AdministrationSystem"))
    emit logging(QString("Administrator - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void ServerManager::on_AdministratorBuilderCompleted_slot() {
  Administrator = AdministratorBuilder->buildedObject();

  // Когда поток завершит работу, составитель заказов будет удален
  connect(AdministratorThread, &QThread::finished, Administrator,
          &QObject::deleteLater);
  // Подключаем логгирование к инициализатору
  connect(Administrator, &AdministrationSystem::logging, this,
          &ServerManager::proxyLogging);
  // Подключаем сигнал для применения новых настроек
  connect(this, &ServerManager::applySettings_signal, Administrator,
          &AdministrationSystem::applySettings);
  // После выполнения операции формирователем заказов, оповещаем менеджер
  connect(Administrator, &AdministrationSystem::operationFinished, this,
          &ServerManager::on_AdministratorFinished_slot);

  // Подключаем функционал
  connect(this, &ServerManager::getDatabaseTable_signal, Administrator,
          &AdministrationSystem::getDatabaseTable);
  connect(this, &ServerManager::getCustomResponse_signal, Administrator,
          &AdministrationSystem::getCustomResponse);
  connect(this, &ServerManager::createNewOrder_signal, Administrator,
          &AdministrationSystem::createNewOrder);
  connect(this, &ServerManager::clearDatabaseTable_signal, Administrator,
          &AdministrationSystem::clearDatabaseTable);
  connect(this, &ServerManager::initIssuerTable_signal, Administrator,
          &AdministrationSystem::initIssuerTable);
  connect(this, &ServerManager::deleteLastOrder_signal, Administrator,
          &AdministrationSystem::deleteLastOrder);
  connect(this, &ServerManager::createNewProductionLine_signal, Administrator,
          &AdministrationSystem::createNewProductionLine);
  connect(this, &ServerManager::deleteLastProductionLines_signal, Administrator,
          &AdministrationSystem::deleteLastProductionLines);
}

void ServerManager::on_ServerThreadFinished_slot() {
  emit logging("Поток сервера завершился. ");
  Host = nullptr;
  ServerThread = nullptr;
}

void ServerManager::on_AdministratorThreadFinished_slot() {
  emit logging("Поток инициализатора завершился. ");
  Administrator = nullptr;
  AdministratorThread = nullptr;
}

void ServerManager::on_AdministratorFinished_slot(
    AdministrationSystem::ExecutionStatus status) {
  switch (status) {
    case AdministrationSystem::NotExecuted:
      CurrentState = Failed;
      NotificarionText = "Инициализатор: операция не была запущена. ";
      emit break;
    case AdministrationSystem::DatabaseConnectionError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: не удалось подключиться к базе данных. ";
      break;
    case AdministrationSystem::DatabaseQueryError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: ошибка при выполнении запроса к базе данных. ";
      break;
    case AdministrationSystem::UnknowError:
      CurrentState = Failed;
      NotificarionText =
          "Инициализатор: получена неизвестная ошибка при выполнении "
          "операции. ";
      break;
    case AdministrationSystem::CompletedSuccessfully:
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
