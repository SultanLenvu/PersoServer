#include "admin_manager.h"
#include "Log/log_system.h"
#include "StickerPrinter/te310_printer.h"

AdminManager::AdminManager(QObject* parent) : QObject(parent) {
  setObjectName("AdminManager");
  loadSettings();
}

AdminManager::~AdminManager() {}

void AdminManager::insctanceThreadStarted_slot() {
  // Создаем администратора
  createAdministrator();

  // Создаем клиента
  createClient();

  // Создаем стикер принтер
  createStickerPrinter();
}

void AdminManager::connectDatabase() {
  // Начинаем выполнение операции
  startOperationPerforming("connectDatabase");

  AdministrationSystem::ReturnStatus status;

  sendLog("Подключение к базе данных. ");
  status = Administrator->connectDatabase();
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "connectDatabase");
    return;
  }

  finishOperationPerforming("connectDatabase");
}

void AdminManager::disconnectDatabase() {
  // Начинаем выполнение операции
  startOperationPerforming("disconnectDatabase");

  AdministrationSystem::ReturnStatus status;

  sendLog("Отключение от базы данных. ");
  status = Administrator->disconnectDatabase();
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "disconnectDatabase");
    return;
  }

  finishOperationPerforming("disconnectDatabase");
}

void AdminManager::showDatabaseTable(const QString& name,
                                     SqlQueryValues* model) {
  // Начинаем выполнение операции
  startOperationPerforming("showDatabaseTable");

  AdministrationSystem::ReturnStatus status;

  model->clear();
  sendLog(QString("Отображение таблицы %1. ").arg(name));
  status = Administrator->getDatabaseTable(name, model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "showDatabaseTable");
    return;
  }

  finishOperationPerforming("showDatabaseTable");
}

void AdminManager::performCustomRequest(const QString& req,
                                        SqlQueryValues* model) {
  // Начинаем выполнение операции
  startOperationPerforming("performCustomRequest");

  AdministrationSystem::ReturnStatus status;

  model->clear();
  sendLog("Представление ответа на кастомный запрос. ");
  status = Administrator->getCustomResponse(req, model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "performCustomRequest");
    return;
  }

  finishOperationPerforming("performCustomRequest");
}

void AdminManager::createNewOrder(
    const std::shared_ptr<QHash<QString, QString>> orderParameters,
    SqlQueryValues* model) {
  startOperationPerforming("createNewOrder");

  AdministrationSystem::ReturnStatus status;

  sendLog("Создание нового заказа. ");
  status = Administrator->createNewOrder(orderParameters);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "createNewOrder");
    return;
  }

  model->clear();
  sendLog("Отображение заказов. ");
  status = Administrator->getDatabaseTable("orders", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "createNewOrder");
    return;
  }

  finishOperationPerforming("createNewOrder");
}

void AdminManager::startOrderAssembling(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("startOrderAssembling");

  AdministrationSystem::ReturnStatus status;

  sendLog(QString("Запуск сборки заказа %1. ").arg(param->value("id")));
  status = Administrator->startOrderAssembling(param->value("id"));
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "startOrderAssembling");
    return;
  }

  model->clear();
  sendLog("Отображение заказов. ");
  status = Administrator->getDatabaseTable("orders", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "startOrderAssembling");
    return;
  }

  finishOperationPerforming("startOrderAssembling");
}

void AdminManager::stopOrderAssembling(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("stopOrderAssembling");

  AdministrationSystem::ReturnStatus status;

  sendLog(QString("Остановка сборки заказа %1. ").arg(param->value("id")));
  status = Administrator->stopOrderAssembling(param->value("id"));
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "stopOrderAssembling");
    return;
  }

  model->clear();
  sendLog("Отображение заказов. ");
  status = Administrator->getDatabaseTable("orders", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "stopOrderAssembling");
    return;
  }

  finishOperationPerforming("stopOrderAssembling");
}

void AdminManager::showOrderTable(SqlQueryValues* model) {
  showDatabaseTable("orders", model);
}

void AdminManager::createNewProductionLine(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("createNewProductionLine");

  AdministrationSystem::ReturnStatus status;

  sendLog("Создание новой линии производства. ");
  status = Administrator->createNewProductionLine(param.get());
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "createNewProductionLine");
    return;
  }

  model->clear();
  sendLog("Отображение производственных линий. ");
  status = Administrator->getDatabaseTable("production_lines", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "createNewProductionLine");
    return;
  }

  finishOperationPerforming("createNewProductionLine");
}

void AdminManager::startProductionLine(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("startProductionLine");

  AdministrationSystem::ReturnStatus status;

  sendLog("Запуск линии производства. ");
  status = Administrator->startProductionLineManually(
      param->value("production_line_id"), param->value("order_id"));
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "startProductionLine");
    return;
  }

  model->clear();
  sendLog("Отображение производственных линий. ");
  status = Administrator->getDatabaseTable("production_lines", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "startProductionLine");
    return;
  }

  finishOperationPerforming("startProductionLine");
}

void AdminManager::stopProductionLine(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("stopProductionLine");

  AdministrationSystem::ReturnStatus status;

  sendLog("Остановка линии производства. ");
  status = Administrator->stopProductionLineManually(param->value("id"));
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "stopProductionLine");
    return;
  }

  model->clear();
  sendLog("Отображение производственных линий. ");
  status = Administrator->getDatabaseTable("production_lines", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "stopProductionLine");
    return;
  }

  finishOperationPerforming("stopProductionLine");
}

void AdminManager::stopAllProductionLines(SqlQueryValues* model) {
  startOperationPerforming("stopAllProductionLines");

  AdministrationSystem::ReturnStatus status;

  sendLog(QString("Остановка всех производственных линий. "));
  status = Administrator->stopAllProductionLinesManually();
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "stopAllProductionLines");
    return;
  }

  model->clear();
  sendLog("Отображение производственных линий. ");
  status = Administrator->getDatabaseTable("production_lines", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "stopAllProductionLines");
    return;
  }

  finishOperationPerforming("stopAllProductionLines");
}

void AdminManager::showProductionLineTable(SqlQueryValues* model) {
  showDatabaseTable("orders", model);
}

void AdminManager::initIssuers(SqlQueryValues* model) {
  startOperationPerforming("initIssuers");

  AdministrationSystem::ReturnStatus status;

  sendLog("Инициализация данных об эмитентах. ");
  status = Administrator->initIssuerTable();
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "initIssuers");
    return;
  }

  model->clear();
  sendLog("Отображение эмитентов. ");
  status = Administrator->getDatabaseTable("issuers", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "initIssuers");
    return;
  }

  finishOperationPerforming("initIssuers");
}

void AdminManager::initTransportMasterKeys(SqlQueryValues* model) {
  startOperationPerforming("initTransportMasterKeys");

  AdministrationSystem::ReturnStatus status;

  sendLog("Инициализация транспортных мастер ключей. ");
  status = Administrator->initTransportMasterKeysTable();
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "initTransportMasterKeys");
    return;
  }

  model->clear();
  sendLog("Отображение транспортных мастер ключей. ");
  status = Administrator->getDatabaseTable("transport_master_keys", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "initTransportMasterKeys");
    return;
  }

  finishOperationPerforming("initTransportMasterKeys");
}

void AdminManager::linkIssuerWithMasterKeys(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("linkIssuerWithMasterKeys");

  AdministrationSystem::ReturnStatus status;

  sendLog(QString("Связывание эмитента %1 с мастер ключами %2. ")
              .arg(param->value("issuer_id"), param->value("key_group_id")));
  status = Administrator->linkIssuerWithMasterKeys(param.get());
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "linkIssuerWithMasterKeys");
    return;
  }

  model->clear();
  sendLog("Отображение таблицы эмитентов. ");
  status = Administrator->getDatabaseTable("issuers", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "linkIssuerWithMasterKeys");
    return;
  }

  finishOperationPerforming("linkIssuerWithMasterKeys");
}

void AdminManager::releaseTranspondersManually(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("releaseTranspondersManually");
  sendLog("Принудительный выпуск транспондеров. ");

  AdministrationSystem::ReturnStatus status;
  QString table = param->value("table");

  if (table == "transponders") {
    status = Administrator->releaseTransponderManually(param->value("id"));
  } else if (table == "boxes") {
    status = Administrator->releaseBoxManually(param->value("id"));
  } else if (table == "pallets") {
    status = Administrator->releasePalletManually(param->value("id"));
  } else if (table == "orders") {
    status = Administrator->releaseOrderManually(param->value("id"));
  } else {
    processAdministratorError(AdministrationSystem::ParameterError,
                              "releaseTranspondersManually");
    return;
  }

  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "releaseTranspondersManually");
    return;
  }

  status = Administrator->getDatabaseTable(param->value("table"), model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "releaseTranspondersManually");
    return;
  }

  finishOperationPerforming("releaseTranspondersManually");
}

void AdminManager::refundTranspondersManually(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("refundTranspondersManually");
  sendLog("Возврат транспондеров. ");

  AdministrationSystem::ReturnStatus status;
  QString table = param->value("table");

  if (table == "transponders") {
    status = Administrator->refundTransponderManually(param->value("id"));
  } else if (table == "boxes") {
    status = Administrator->refundBoxManually(param->value("id"));
  } else if (table == "pallets") {
    status = Administrator->refundPalletManually(param->value("id"));
  } else if (table == "orders") {
    status = Administrator->refundOrderManually(param->value("id"));
  } else {
    processAdministratorError(AdministrationSystem::ParameterError,
                              "refundTranspondersManually");
    return;
  }

  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "refundTranspondersManually");
    return;
  }

  status = Administrator->getDatabaseTable(param->value("table"), model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "refundTranspondersManually");
    return;
  }

  finishOperationPerforming("refundTranspondersManually");
}

void AdminManager::shipPallets(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("shipPallets");
  sendLog("Отгрузка паллет. ");

  AdministrationSystem::ReturnStatus status;
  status = Administrator->shipPallets(param.get());
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "shipPallets");
    return;
  }

  status = Administrator->getDatabaseTable("pallets", model);
  if (status != AdministrationSystem::Completed) {
    processAdministratorError(status, "shipPallets");
    return;
  }

  finishOperationPerforming("shipPallets");
}

void AdminManager::releaseTransponder(
    const std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("releaseTransponder");
  sendLog("Выпуск транспондера. ");

  std::shared_ptr<QFile> firmware(new QFile("temp.bin"));
  std::shared_ptr<QHash<QString, QString>> transponderData(
      new QHash<QString, QString>());
  PersoClient::ReturnStatus status = Client->requestTransponderRelease(
      param.get(), firmware.get(), transponderData.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "releaseTransponder");
    return;
  }

  sendLog("Запрос на отображение полученой прошивки транспондера. ");
  emit displayFirmware_signal(firmware);

  sendLog("Запрос на отображение данных транспондера. ");
  emit displayTransponderData_signal(transponderData);

  finishOperationPerforming("releaseTransponder");
}

void AdminManager::confirmTransponderRelease(
    const std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("confirmTransponderRelease");
  sendLog("Подтверждение выпуска транспондера. ");

  PersoClient::ReturnStatus status =
      Client->requestTransponderReleaseConfirm(param.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "confirmTransponderRelease");
    return;
  }

  finishOperationPerforming("confirmTransponderRelease");
}

void AdminManager::rereleaseTransponder(
    const std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("rereleaseTransponder");
  sendLog("Перевыпуск транспондера. ");

  std::shared_ptr<QFile> firmware(new QFile("temp.bin"));
  std::shared_ptr<QHash<QString, QString>> transponderData(
      new QHash<QString, QString>());
  PersoClient::ReturnStatus status = Client->requestTransponderRerelease(
      param.get(), firmware.get(), transponderData.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "releaseTransponder");
    return;
  }

  sendLog("Запрос на отображение полученой прошивки транспондера. ");
  emit displayFirmware_signal(firmware);
  sendLog("Запрос на отображение данных транспондера. ");
  emit displayTransponderData_signal(transponderData);

  finishOperationPerforming("rereleaseTransponder");
}

void AdminManager::confirmTransponderRerelease(
    const std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("confirmTransponderRerelease");
  sendLog("Подтверждение перевыпуска транспондера. ");

  PersoClient::ReturnStatus status =
      Client->requestTransponderRereleaseConfirm(param.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "confirmTransponderRelease");
    return;
  }

  finishOperationPerforming("confirmTransponderRerelease");
}

void AdminManager::rollbackProductionLine(
    const std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("rollbackProductionLine");
  sendLog("Откат производственной линии. ");

  PersoClient::ReturnStatus status =
      Client->requestProductionLineRollback(param.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "rollbackProductionLine");
    return;
  }

  finishOperationPerforming("rollbackProductionLine");
}

void AdminManager::printBoxStickerOnServer(
    std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("printBoxStickerOnServer");
  sendLog("Печать стикера для бокса на сервере. ");

  PersoClient::ReturnStatus status =
      Client->requestBoxStickerPrint(param.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "printBoxStickerOnServer");
    return;
  }

  finishOperationPerforming("printBoxStickerOnServer");
}

void AdminManager::printLastBoxStickerOnServer() {
  startOperationPerforming("printLastBoxStickerOnServer");
  sendLog("Повторная печать последнего стикера для бокса на сервере. ");

  PersoClient::ReturnStatus status = Client->requestBoxStickerReprint();
  if (status != PersoClient::Completed) {
    processClientError(status, "printLastBoxStickerOnServer");
    return;
  }

  finishOperationPerforming("printLastBoxStickerOnServer");
}

void AdminManager::printPalletStickerOnServer(
    std::shared_ptr<QHash<QString, QString>> param) {
  startOperationPerforming("printPalletStickerOnServer");
  sendLog("Печать стикера для паллеты на сервере. ");

  PersoClient::ReturnStatus status =
      Client->requestPalletStickerPrint(param.get());
  if (status != PersoClient::Completed) {
    processClientError(status, "printPalletStickerOnServer");
    return;
  }

  finishOperationPerforming("printPalletStickerOnServer");
}

void AdminManager::printLastPalletStickerOnServer() {
  startOperationPerforming("printLastPalletStickerOnServer");
  sendLog("Повторная печать последнего стикера для паллеты на сервере. ");

  PersoClient::ReturnStatus status = Client->requestPalletStickerReprint();
  if (status != PersoClient::Completed) {
    processClientError(status, "printLastPalletStickerOnServer");
    return;
  }

  finishOperationPerforming("printLastPalletStickerOnServer");
}

void AdminManager::printTransponderSticker(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("printTransponderSticker");

  IStickerPrinter::ReturnStatus stickerPrinterStatus;
  AdministrationSystem::ReturnStatus administratorStatus;

  QHash<QString, QString> transponderData;
  sendLog("Запрос данных транспондера. ");
  administratorStatus =
      Administrator->getTransponderData(param->value("id"), &transponderData);
  if (administratorStatus != AdministrationSystem::Completed) {
    processAdministratorError(administratorStatus, "printTransponderSticker");
    return;
  }

  sendLog("Печать стикера транспондера. ");
  stickerPrinterStatus =
      StickerPrinter->printTransponderSticker(&transponderData);
  if (stickerPrinterStatus != IStickerPrinter::Completed) {
    processStickerPrinterError(stickerPrinterStatus, "printTransponderSticker");
    return;
  }

  finishOperationPerforming("printTransponderSticker");
}

void AdminManager::printBoxSticker(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("printBoxSticker");

  IStickerPrinter::ReturnStatus stickerPrinterStatus;
  AdministrationSystem::ReturnStatus administratorStatus;

  QHash<QString, QString> boxData;
  sendLog("Запрос данных бокса. ");
  administratorStatus = Administrator->getBoxData(param->value("id"), &boxData);
  if (administratorStatus != AdministrationSystem::Completed) {
    processAdministratorError(administratorStatus, "printBoxSticker");
    return;
  }

  sendLog("Печать стикера бокса. ");
  stickerPrinterStatus = StickerPrinter->printBoxSticker(&boxData);
  if (stickerPrinterStatus != IStickerPrinter::Completed) {
    processStickerPrinterError(stickerPrinterStatus, "printBoxSticker");
    return;
  }

  finishOperationPerforming("printBoxSticker");
}

void AdminManager::printPalletSticker(
    const std::shared_ptr<QHash<QString, QString>> param,
    SqlQueryValues* model) {
  startOperationPerforming("printPalletSticker");

  IStickerPrinter::ReturnStatus stickerPrinterStatus;
  AdministrationSystem::ReturnStatus administratorStatus;

  QHash<QString, QString> palletData;
  sendLog("Запрос данных паллеты. ");
  administratorStatus =
      Administrator->getPalletData(param->value("id"), &palletData);
  if (administratorStatus != AdministrationSystem::Completed) {
    processAdministratorError(administratorStatus, "printTransponderSticker");
    return;
  }

  sendLog("Печать стикера паллеты. ");
  stickerPrinterStatus = StickerPrinter->printPalletSticker(&palletData);
  if (stickerPrinterStatus != IStickerPrinter::Completed) {
    processStickerPrinterError(stickerPrinterStatus, "printPalletSticker");
    return;
  }

  finishOperationPerforming("printPalletSticker");
}

void AdminManager::execPrinterStickerCommandScript(
    const std::shared_ptr<QStringList> commandScript) {
  startOperationPerforming("execPrinterStickerCommandScript");

  IStickerPrinter::ReturnStatus stickerPrinterStatus;

  sendLog("Выполнение командного скрипта принтера стикеров. ");
  stickerPrinterStatus = StickerPrinter->exec(commandScript.get());
  if (stickerPrinterStatus != IStickerPrinter::Completed) {
    processStickerPrinterError(stickerPrinterStatus,
                               "execPrinterStickerCommandScript");
    return;
  }

  finishOperationPerforming("execPrinterStickerCommandScript");
}

void AdminManager::applySettings() {
  sendLog("Применение новых настроек. ");
  loadSettings();

  Administrator->applySettings();
  Client->applySettings();
  StickerPrinter->applySetting();
}

void AdminManager::loadSettings() {
  QSettings settings;

  LogEnable = settings.value("log_system/global_enable").toBool();
}

void AdminManager::sendLog(const QString& log) const {
  if (LogEnable) {
    emit const_cast<AdminManager*>(this)->logging(
        QString("%1 - %2").arg(objectName(), log));
  }
}

void AdminManager::createAdministrator() {
  Administrator = new AdministrationSystem(this);
  connect(Administrator, &AdministrationSystem::logging, LogSystem::instance(),
          &LogSystem::generate);

  // Заполняем таблицу соответствий статусов возврата
  AdministratorReturnStatusMatch.insert(AdministrationSystem::NotExecuted,
                                        "Выполнение операции не началось.");
  AdministratorReturnStatusMatch.insert(AdministrationSystem::ParameterError,
                                        "Получены некорректные параметры.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::DatabaseConnectionError,
      "Не удалось подключиться к базе данных. ");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::DatabaseTransactionError, "Ошибка транзакции. ");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::DatabaseQueryError,
      "Получена ошибка при выполнении запроса к базе данных.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::RegisterFileError,
      "Не удалось открыть файл-реестр для отгрузки.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::ProductionLineMissed,
      "Не удалось найти производственную линию.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::ProductionLineLinkError,
      "Не удалось связать производственную линию с боксом.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::ProductionLineRollbackLimit,
      "Производственная линия находится на первом транспондере в боксе.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::OrderRemovingError, "Не удалось удалить заказ.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::OtherOrderInProcess,
      "В процессе сборки находится другой заказ.");
  AdministratorReturnStatusMatch.insert(AdministrationSystem::OrderNotInProcess,
                                        "Сборка заказа не была запущена.");
  AdministratorReturnStatusMatch.insert(
      AdministrationSystem::MultipleActiveOrders,
      "В процессе сборки находится несколько заказов.");
  AdministratorReturnStatusMatch.insert(AdministrationSystem::FreeBoxMissed,
                                        "Не удалось найти свободный бокс.");
  AdministratorReturnStatusMatch.insert(AdministrationSystem::UnknownError,
                                        "Неизвествная ошибка. ");
  AdministratorReturnStatusMatch.insert(AdministrationSystem::Completed,
                                        "Выполнено. ");
}

void AdminManager::createClient() {
  Client = new PersoClient(this);
  connect(Client, &PersoClient::logging, LogSystem::instance(),
          &LogSystem::generate);

  // Заполняем таблицу соответствий статусов возврата
  ClientReturnStatusMatch.insert(PersoClient::Completed, "Выполнено. ");
  ClientReturnStatusMatch.insert(PersoClient::RequestParameterError,
                                 "Не удалось обработать параметры запроса. ");
  ClientReturnStatusMatch.insert(PersoClient::ServerConnectionError,
                                 "Не удалось подключиться к серверу. ");
  ClientReturnStatusMatch.insert(PersoClient::ServerNotResponding,
                                 "Сервер не отвечает.");
  ClientReturnStatusMatch.insert(PersoClient::ServerConnectionTerminated,
                                 "Оборвалось соединение с сервером.");
  ClientReturnStatusMatch.insert(PersoClient::AuthorizationNotExist,
                                 "Производственная линия не найдена. ");
  ClientReturnStatusMatch.insert(PersoClient::AuthorizationAccessDenied,
                                 "Ошибка доступа к производственной линии.");
  ClientReturnStatusMatch.insert(PersoClient::AuthorizationNotActive,
                                 "Производственная линия не активна. ");
  ClientReturnStatusMatch.insert(PersoClient::ResponseSyntaxError,
                                 "Синтаксическая ошибка в ответе на запрос. ");
  ClientReturnStatusMatch.insert(
      PersoClient::CommandSyntaxError,
      "В серверном запросе допущена синтаксическая ошибка. ");
  ClientReturnStatusMatch.insert(
      PersoClient::DatabaseError,
      "Получена ошибка при выполнении запроса в базу данных. ");
  ClientReturnStatusMatch.insert(PersoClient::TransponderNotFound,
                                 "Транспондер не найден.");
  ClientReturnStatusMatch.insert(
      PersoClient::TransponderNotReleasedEarlier,
      "Транспондер не был выпущен ранее, перевыпуск невозможен. ");
  ClientReturnStatusMatch.insert(PersoClient::AwaitingConfirmationError,
                                 "Транспондер не ожидает подтверждения. ");
  ClientReturnStatusMatch.insert(PersoClient::IdenticalUcidError,
                                 "Используется одна и та же печатная плата. "
                                 "Перевыпуск транспондера невозможен. ");
  ClientReturnStatusMatch.insert(PersoClient::ProductionLineMissed,
                                 "Производственная линия не найдена. ");
  ClientReturnStatusMatch.insert(PersoClient::ProductionLineNotActive,
                                 "Производственная линия не активна. ");
  ClientReturnStatusMatch.insert(
      PersoClient::CurrentOrderRunOut,
      "В текущем заказе отсутствуют свободные транспондеры . ");
  ClientReturnStatusMatch.insert(PersoClient::CurrentOrderAssembled,
                                 "Текущий заказ собран. ");
  ClientReturnStatusMatch.insert(PersoClient::ProductionLineRollbackLimitError,
                                 "Производственная линия связана с первым "
                                 "транспондером в боксе. Откат невозможен. ");
  ClientReturnStatusMatch.insert(PersoClient::BoxStickerPrintError,
                                 "Не удалось распечатать стикер для бокса. ");
  ClientReturnStatusMatch.insert(PersoClient::PalletStickerPrintError,
                                 "Не удалось распечатать стикер для паллеты. ");
  ClientReturnStatusMatch.insert(
      PersoClient::NextTransponderNotFound,
      "Получена ошибка при поиске очередного транспондера. ");
  ClientReturnStatusMatch.insert(
      PersoClient::StartBoxAssemblingError,
      "Получена ошибка при запуске сборки очередного бокса. ");
  ClientReturnStatusMatch.insert(
      PersoClient::StartPalletAssemblingError,
      "Получена ошибка при запуске сборки очередной паллеты. ");
}

void AdminManager::createStickerPrinter() {
  StickerPrinter = new TE310Printer(this, "RandomPrinter");
  connect(StickerPrinter, &IStickerPrinter::logging, LogSystem::instance(),
          &LogSystem::generate);

  // Заполняем таблицу соответствий статусов возврата
  StickerPrinterReturnStatusMatch.insert(
      IStickerPrinter::ParameterError,
      "Получены некорректные параметры для стикера.");
  StickerPrinterReturnStatusMatch.insert(IStickerPrinter::Failed,
                                         "Не удалось распечать стикер.");
  StickerPrinterReturnStatusMatch.insert(
      IStickerPrinter::LibraryMissed,
      "Отсутствует библиотека для работы с принтером стикеров.");
  StickerPrinterReturnStatusMatch.insert(IStickerPrinter::ConnectionError,
                                         "Не удалось подключиться к принтеру.");
  StickerPrinterReturnStatusMatch.insert(IStickerPrinter::Completed,
                                         "Выполнено.");
}

void AdminManager::startOperationPerforming(const QString& operationName) {
  Mutex.lock();

  emit operationPerfomingStarted(operationName);
}

void AdminManager::finishOperationPerforming(const QString& operationName) {
  // Сигнал о завершении текущей операции
  emit operationPerformingFinished(operationName);

  // Оповещаем пользователя
  emit notifyUser("Выполнено.");

  // Разблокируем мьютекс
  Mutex.unlock();
}

void AdminManager::processAdministratorError(
    AdministrationSystem::ReturnStatus status,
    const QString& operationName) {
  sendLog(AdministratorReturnStatusMatch.value(status));
  emit operationPerformingFinished(operationName);
  emit notifyUserAboutError(AdministratorReturnStatusMatch.value(status));
  Mutex.unlock();
}

void AdminManager::processClientError(PersoClient::ReturnStatus status,
                                      const QString& operationName) {
  sendLog(ClientReturnStatusMatch.value(status));
  emit operationPerformingFinished(operationName);
  emit notifyUserAboutError(ClientReturnStatusMatch.value(status));
  Mutex.unlock();
}

void AdminManager::processStickerPrinterError(
    IStickerPrinter::ReturnStatus status,
    const QString& operationName) {
  sendLog(StickerPrinterReturnStatusMatch.value(status));
  emit operationPerformingFinished(operationName);
  emit notifyUserAboutError(StickerPrinterReturnStatusMatch.value(status));
  Mutex.unlock();
}

//==================================================================================
