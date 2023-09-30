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

void ServerManager::startServer() {
  if ((Host) && (Host->isListening())) {
    emit logging("Сервер уже запущен. ");
    return;
  }

  emit logging("Запуск сервера персонализации. ");
  emit startServer_signal();
}

void ServerManager::stopServer() {
  emit logging("Остановка сервера персонализации. ");
  emit stopServer_signal();
}

void ServerManager::connectDatabaseManually() {
  // Начинаем выполнение операции
  if (!startOperationExecution("connectDatabaseManually")) {
    return;
  }

  emit logging("Прямое подключение к базе данных. ");
  emit connectDatabase_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("connectDatabaseManually");
}

void ServerManager::disconnectDatabaseManually() {
  // Начинаем выполнение операции
  if (!startOperationExecution("disconnectDatabaseManually")) {
    return;
  }

  emit logging("Прямое отключение от базы данных. ");
  emit disconnectDatabase_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("disconnectDatabaseManually");
}

void ServerManager::showDatabaseTable(const QString& name,
                                      DatabaseTableModel* model) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showDatabaseTable")) {
    return;
  }

  model->clear();
  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal(name, model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showDatabaseTable");
}

void ServerManager::clearDatabaseTable(const QString& name,
                                       DatabaseTableModel* model) {
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

  model->clear();
  emit logging("Отображение таблицы базы данных. ");
  emit getDatabaseTable_signal(name, model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("clearDatabaseTable");
}

void ServerManager::performCustomRequest(const QString& req,
                                         DatabaseTableModel* model) {
  // Начинаем выполнение операции
  if (!startOperationExecution("performCustomRequest")) {
    return;
  }

  model->clear();
  emit logging("Представление ответа на кастомный запрос. ");
  emit getCustomResponse_signal(req, model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("performCustomRequest");
}

void ServerManager::createNewOrder(
    const QMap<QString, QString>* orderParameters,
    DatabaseTableModel* model) {
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

  model->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewOrder");
}

void ServerManager::startOrderAssemblingManually(const QString& orderId,
                                                 DatabaseTableModel* model) {
  if (!startOperationExecution("startOrderAssemblingManually")) {
    return;
  }

  emit logging(QString("Запуск сборки заказа %1. ").arg(orderId));
  emit startOrderAssembling_signal(orderId);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("startOrderAssemblingManually");
    return;
  }

  model->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("startOrderAssemblingManually");
}

void ServerManager::stopOrderAssemblingManually(const QString& orderId,
                                                DatabaseTableModel* model) {
  if (!startOperationExecution("startOrderAssemblingManually")) {
    return;
  }

  emit logging(QString("Остановка сборки заказа %1. ").arg(orderId));
  emit stopOrderAssembling_signal(orderId);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("startOrderAssemblingManually");
    return;
  }

  model->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("startOrderAssemblingManually");
}

void ServerManager::deleteLastOrder(DatabaseTableModel* model) {
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

  model->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewOrder");
}

void ServerManager::showOrderTable(DatabaseTableModel* model) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showOrderTable")) {
    return;
  }

  model->clear();
  emit logging("Отображение заказов. ");
  emit getDatabaseTable_signal("orders", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showOrderTable");
}

void ServerManager::createNewProductionLine(
    const QMap<QString, QString>* productionLineParameters,
    DatabaseTableModel* model) {
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

  model->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("createNewProductionLine");
}

void ServerManager::allocateInactiveProductionLinesManually(
    const QString& orderId,
    DatabaseTableModel* model) {
  if (!startOperationExecution("allocateInactiveProductionLinesManually")) {
    return;
  }

  emit logging(
      QString("Распределение неактивных линий производства в заказе %1. ")
          .arg(orderId));
  emit allocateInactiveProductionLines_signal(orderId);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("allocateInactiveProductionLinesManually");
    return;
  }

  model->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("allocateInactiveProductionLinesManually");
}

void ServerManager::shutdownAllProductionLinesManually(
    DatabaseTableModel* model) {
  if (!startOperationExecution("shutdownAllProductionLinesManually")) {
    return;
  }

  emit logging(QString("Остановка всех производственных линий. "));
  emit shutdownAllProductionLines_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("shutdownAllProductionLinesManually");
    return;
  }

  model->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("shutdownAllProductionLinesManually");
}

void ServerManager::deleteLastProductionLine(DatabaseTableModel* model) {
  if (!startOperationExecution("deleteLastProductionLine")) {
    return;
  }

  emit logging("Удаление последней линии производства. ");
  emit removeLastProductionLine_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("deleteLastProductionLine");
    return;
  }

  model->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("deleteLastProductionLine");
}

void ServerManager::showProductionLineTable(DatabaseTableModel* model) {
  // Начинаем выполнение операции
  if (!startOperationExecution("showProductionLineTable")) {
    return;
  }

  model->clear();
  emit logging("Отображение линий производства. ");
  emit getDatabaseTable_signal("production_lines", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("showProductionLineTable");
}

void ServerManager::linkProductionLineWithBoxManually(
    const QMap<QString, QString>* parameters,
    DatabaseTableModel* model) {
  if (!startOperationExecution("linkProductionLineWithBoxManually")) {
    return;
  }

  emit logging("Связывание линии производства с определенным боксом. ");
  emit linkProductionLineWithBox_signal(parameters);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Проверка состояния
  if (CurrentState == Failed) {
    // Завершаем выполнение операции
    endOperationExecution("linkProductionLineWithBoxManually");
    return;
  }

  model->clear();
  emit logging("Отображение производственных линий. ");
  emit getDatabaseTable_signal("production_lines", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("linkProductionLineWithBoxManually");
}

void ServerManager::releaseTransponderManually(
    const QMap<QString, QString>* releaseParameters,
    TransponderSeedModel* seed) {
  // Начинаем выполнение операции
  if (!startOperationExecution("releaseTransponderManually")) {
    return;
  }

  emit logging("Выпуск транспондера. ");
  QMap<QString, QString>* attributes = new QMap<QString, QString>();
  QMap<QString, QString>* masterKeys = new QMap<QString, QString>();
  emit releaseTransponder_signal(releaseParameters, attributes, masterKeys);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  seed->build(attributes, masterKeys);
  endOperationExecution("releaseTransponderManually");
}

void ServerManager::confirmTransponderReleaseManually(
    const QMap<QString, QString>* confirmParameters,
    TransponderSeedModel* seed) {
  // Начинаем выполнение операции
  if (!startOperationExecution("confirmTransponderReleaseManually")) {
    return;
  }

  emit logging("Подтверждение выпуска транспондера. ");
  QMap<QString, QString>* attributes = new QMap<QString, QString>();
  QMap<QString, QString>* masterKeys = new QMap<QString, QString>();
  emit confirmReleaseTransponder_signal(confirmParameters, attributes);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  seed->build(attributes, masterKeys);
  endOperationExecution("confirmTransponderReleaseManually");
}

void ServerManager::rereleaseTransponderManually(
    const QMap<QString, QString>* rereleaseParameters,
    TransponderSeedModel* seed) {
  // Начинаем выполнение операции
  if (!startOperationExecution("rereleaseTransponderManually")) {
    return;
  }

  emit logging("Перевыпуск транспондера. ");
  QMap<QString, QString>* attributes = new QMap<QString, QString>();
  QMap<QString, QString>* masterKeys = new QMap<QString, QString>();
  emit rereleaseTransponder_signal(rereleaseParameters, attributes, masterKeys);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  seed->build(attributes, masterKeys);
  endOperationExecution("rereleaseTransponderManually");
}

void ServerManager::confirmTransponderRereleaseManually(
    const QMap<QString, QString>* confirmParameters,
    TransponderSeedModel* seed) {
  // Начинаем выполнение операции
  if (!startOperationExecution("confirmTransponderRereleaseManually")) {
    return;
  }

  emit logging("Подтверждение перевыпуска транспондера. ");
  QMap<QString, QString>* attributes = new QMap<QString, QString>();
  QMap<QString, QString>* masterKeys = new QMap<QString, QString>();
  emit confirmRereleaseTransponder_signal(confirmParameters, attributes);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  seed->build(attributes, masterKeys);
  endOperationExecution("confirmTransponderRereleaseManually");
}

void ServerManager::searchTransponderManually(
    const QMap<QString, QString>* searchParameters,
    TransponderSeedModel* seed) {
  // Начинаем выполнение операции
  if (!startOperationExecution("searchTransponderManually")) {
    return;
  }

  emit logging("Выпуск транспондера. ");
  emit searchTransponder_signal(searchParameters, seed);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("searchTransponderManually");
}

void ServerManager::refundTransponderManually(
    const QMap<QString, QString>* refundParameters,
    TransponderSeedModel* seed) {
  // Начинаем выполнение операции
  if (!startOperationExecution("refundTransponderManually")) {
    return;
  }

  emit logging("Возврат транспондера. ");
  emit refundTransponder_signal(refundParameters, seed);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("refundTransponderManually");
}

void ServerManager::initIssuers(DatabaseTableModel* model) {
  if (!startOperationExecution("initIssuers")) {
    return;
  }

  emit logging("Инициализация данных об эмитентах. ");
  emit initIssuerTable_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  if (CurrentState != Completed) {
    // Завершаем выполнение операции
    endOperationExecution("initIssuers");
    return;
  }

  model->clear();
  emit logging("Отображение эмитентов. ");
  emit getDatabaseTable_signal("issuers", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("initIssuers");
}

void ServerManager::initTransportMasterKeys(DatabaseTableModel* model) {
  if (!startOperationExecution("initTransportMasterKeys")) {
    return;
  }

  emit logging("Инициализация транспортных мастер ключей. ");
  emit initTransportMasterKeysTable_signal();

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  if (CurrentState != Completed) {
    // Завершаем выполнение операции
    endOperationExecution("initTransportMasterKeys");
    return;
  }

  model->clear();
  emit logging("Отображение транспортных мастер ключей. ");
  emit getDatabaseTable_signal("transport_master_keys", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("initTransportMasterKeys");
}

void ServerManager::linkIssuerWithMasterKeys(
    DatabaseTableModel* model,
    const QMap<QString, QString>* parameters) {
  if (!startOperationExecution("linkIssuerWithMasterKeys")) {
    return;
  }

  emit logging(QString("Связывание эмитента %1 с мастер ключами %2. ")
                   .arg(parameters->value("issuer_id"),
                        parameters->value("master_keys_id")));
  emit linkIssuerWithMasterKeys_signal(parameters);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  if (CurrentState != Completed) {
    // Завершаем выполнение операции
    endOperationExecution("linkIssuerWithMasterKeys");
    return;
  }

  model->clear();
  emit logging("Отображение эмитентов. ");
  emit getDatabaseTable_signal("issuers", model);

  // Запускаем цикл ожидания
  WaitingLoop->exec();

  // Завершаем выполнение операции
  endOperationExecution("linkIssuerWithMasterKeys");
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
  connect(this, &ServerManager::startServer_signal, Host, &PersoHost::start);
  connect(this, &ServerManager::stopServer_signal, Host, &PersoHost::stop);

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
                    QVariant::fromValue(duration));

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
  connect(this, &ServerManager::connectDatabase_signal, Administrator,
          &AdministrationSystem::connectDatabase);
  connect(this, &ServerManager::disconnectDatabase_signal, Administrator,
          &AdministrationSystem::disconnectDatabase);
  connect(this, &ServerManager::getDatabaseTable_signal, Administrator,
          &AdministrationSystem::getDatabaseTable);
  connect(this, &ServerManager::getCustomResponse_signal, Administrator,
          &AdministrationSystem::getCustomResponse);
  connect(this, &ServerManager::clearDatabaseTable_signal, Administrator,
          &AdministrationSystem::clearDatabaseTable);

  connect(this, &ServerManager::createNewOrder_signal, Administrator,
          &AdministrationSystem::createNewOrder);
  connect(this, &ServerManager::startOrderAssembling_signal, Administrator,
          &AdministrationSystem::startOrderAssembling);
  connect(this, &ServerManager::stopOrderAssembling_signal, Administrator,
          &AdministrationSystem::stopOrderAssembling);
  connect(this, &ServerManager::deleteLastOrder_signal, Administrator,
          &AdministrationSystem::deleteLastOrder);

  connect(this, &ServerManager::createNewProductionLine_signal, Administrator,
          &AdministrationSystem::createNewProductionLine);
  connect(this, &ServerManager::allocateInactiveProductionLines_signal,
          Administrator,
          &AdministrationSystem::allocateInactiveProductionLines);
  connect(this, &ServerManager::shutdownAllProductionLines_signal,
          Administrator, &AdministrationSystem::shutdownAllProductionLines);
  connect(this, &ServerManager::removeLastProductionLine_signal, Administrator,
          &AdministrationSystem::deleteLastProductionLine);
  connect(this, &ServerManager::linkProductionLineWithBox_signal, Administrator,
          &AdministrationSystem::linkProductionLineWithBox);

  connect(this, &ServerManager::releaseTransponder_signal, Administrator,
          &AdministrationSystem::releaseTransponder);
  connect(this, &ServerManager::confirmReleaseTransponder_signal, Administrator,
          &AdministrationSystem::confirmReleaseTransponder);
  connect(this, &ServerManager::rereleaseTransponder_signal, Administrator,
          &AdministrationSystem::rereleaseTransponder);
  connect(this, &ServerManager::confirmRereleaseTransponder_signal,
          Administrator, &AdministrationSystem::confirmRereleaseTransponder);
  //  connect(this, &ServerManager::refundTransponder_signal, Administrator,
  //          &AdministrationSystem::refundTransponder);
  connect(this, &ServerManager::searchTransponder_signal, Administrator,
          &AdministrationSystem::searchTransponder);

  connect(this, &ServerManager::initIssuerTable_signal, Administrator,
          &AdministrationSystem::initIssuerTable);
  connect(this, &ServerManager::initTransportMasterKeysTable_signal,
          Administrator, &AdministrationSystem::initTransportMasterKeysTable);
  connect(this, &ServerManager::linkIssuerWithMasterKeys_signal, Administrator,
          &AdministrationSystem::linkIssuerWithMasterKeys);
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
      NotificarionText = "Администратор: операция не была запущена. ";
      break;
    case AdministrationSystem::DatabaseConnectionError:
      CurrentState = Failed;
      NotificarionText =
          "Администратор: не удалось подключиться к базе данных. ";
      break;
    case AdministrationSystem::DatabaseQueryError:
      CurrentState = Failed;
      NotificarionText =
          "Администратор: ошибка при выполнении запроса к базе данных. ";
      break;
    case AdministrationSystem::LogicError:
      CurrentState = Failed;
      NotificarionText = "Администратор: получена логическая ошибка. ";
      break;
    case AdministrationSystem::ReleaserError:
      CurrentState = Failed;
      NotificarionText =
          "Администратор: получена ошибка в системе выпуска "
          "транспондеров. ";
      break;
    case AdministrationSystem::UnknowError:
      CurrentState = Failed;
      NotificarionText =
          "Администратор: получена неизвестная ошибка при выполнении "
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
