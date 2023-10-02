#include "main_window_kernel.h"

MainWindowKernel::MainWindowKernel(QWidget* parent) : QMainWindow(parent) {
  // Считываем размеры дисплея
  DesktopGeometry = QApplication::desktop()->screenGeometry();

  // Загружаем пользовательские настройки
  loadSettings();

  // Графический интерфейс пока не создан
  CurrentGUI = nullptr;
  Manager = nullptr;
  Logger = nullptr;
  Interactor = nullptr;

  // Создаем систему логгирования
  setupLogger();

  // Создаем графический интерфейс окна начальной конфигурации
  on_OpenInitialGuiRequestAct_slot();

  // Cистема взаимодействия с пользователем
  setupInterructionSystem();

  // Управляющий модуль
  setupManager();

  // Создаем модели для представлений
  createModels();

  // Создаем таблицу соответствий
  createMatchingTable();
}

MainWindowKernel::~MainWindowKernel() {}

ServerManager* MainWindowKernel::manager() {
  return Manager;
}

void MainWindowKernel::on_OpenMasterGuiPushButton_slot() {
  // Запрашиваем пароль
  //  QString key;
  //  Interactor->getMasterPassword(key);

  //  if (key == MASTER_ACCESS_PASSWORD) {
  createMasterInterface();
  connectMasterInterface();
  //  } else {
  //    Interactor->generateError("Неверный код доступа");
  //  }
}

void MainWindowKernel::on_OpenInitialGuiRequestAct_slot() {
  createInitialInterface();
  connectInitialInterface();
}

void MainWindowKernel::on_ServerStartPushButton_slot() {
  Logger->clear();
  Manager->startServer();
}

void MainWindowKernel::on_ServerStopPushButton_slot() {
  Logger->clear();
  Manager->stopServer();
}

void MainWindowKernel::on_ConnectDatabasePushButton_slot() {
  Logger->clear();
  Manager->connectDatabaseManually();
}

void MainWindowKernel::on_DisconnectDatabasePushButton_slot() {
  Logger->clear();
  Manager->disconnectDatabaseManually();
}

void MainWindowKernel::on_ShowDatabaseTablePushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  Manager->showDatabaseTable(gui->DatabaseTableChoice->currentText(),
                             RandomModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_ClearDatabaseTablePushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  Manager->clearDatabaseTable(gui->DatabaseTableChoice->currentText(),
                              RandomModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_TransmitCustomRequestPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  Manager->performCustomRequest(gui->CustomRequestLineEdit->text(),
                                RandomModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_CreateNewOrderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  Logger->clear();

  if (!checkNewOrderInput()) {
    Interactor->generateError("Некорректный ввод параметров нового заказа. ");
    return;
  }

  QMap<QString, QString> orderParameters;
  orderParameters.insert("issuer_name", gui->IssuerNameComboBox->currentText());
  orderParameters.insert("transponder_quantity",
                         gui->TransponderQuantityLineEdit->text());
  orderParameters.insert("box_capacity", gui->BoxCapacityLineEdit->text());
  orderParameters.insert("pallet_capacity",
                         gui->PalletCapacityLineEdit->text());
  orderParameters.insert(
      "full_personalization",
      gui->FullPersonalizationCheckBox->checkState() == Qt::Checked ? "true"
                                                                    : "false");
  orderParameters.insert("pan_file_path", gui->PanFilePathLineEdit->text());
  orderParameters.insert("transponder_model",
                         gui->TransponderModelLineEdit->text());
  orderParameters.insert("accr_reference", gui->AccrReferenceLineEdit->text());
  orderParameters.insert("equipment_class",
                         gui->EquipmentClassLineEdit->text());
  orderParameters.insert("manufacturer_id",
                         gui->ManufacturerIdLineEdit->text());

  Manager->createNewOrder(&orderParameters, OrderModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_StartOrderAssemblingPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  if (gui->OrderIdLineEdit1->text().toInt() == 0) {
    Interactor->generateError("Некорректный ввод идентификатора заказа. ");
    return;
  }

  Manager->startOrderAssemblingManually(gui->OrderIdLineEdit1->text(),
                                        OrderModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_StopOrderAssemblingPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  if (gui->OrderIdLineEdit1->text().toInt() == 0) {
    Interactor->generateError("Некорректный ввод идентификатора заказа. ");
    return;
  }

  Manager->stopOrderAssemblingManually(gui->OrderIdLineEdit1->text(),
                                       OrderModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_UpdateOrderViewPushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("orders", OrderModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_DeleteLastOrderPushButton_slot() {
  Logger->clear();

  Manager->deleteLastOrder(OrderModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_CreateNewProductionLinePushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  if (!checkNewProductionLineInput()) {
    Interactor->generateError(
        "Некорректный ввод параметров нового производственной линии. ");
    return;
  }

  QMap<QString, QString> productionLineParameters;
  productionLineParameters.insert("login", gui->LoginLineEdit1->text());
  productionLineParameters.insert("password", gui->PasswordLineEdit1->text());
  Manager->createNewProductionLine(&productionLineParameters,
                                   ProductionLineModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_AllocateInactiveProductionLinesPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  if (gui->OrderIdLineEdit2->text().toInt() == 0) {
    Interactor->generateError("Некорректный ввод идентификатора заказа. ");
    return;
  }

  Manager->allocateInactiveProductionLinesManually(
      gui->OrderIdLineEdit2->text(), ProductionLineModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_LinkProductionLinePushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  Logger->clear();

  if ((!checkNewProductionLineInput()) ||
      (gui->BoxIdLineEdit->text().toInt() == 0)) {
    Interactor->generateError(
        "Некорректный ввод параметров производственной линии. ");
    return;
  }

  QMap<QString, QString> linkParameters;
  linkParameters.insert("login", gui->LoginLineEdit1->text());
  linkParameters.insert("password", gui->PasswordLineEdit1->text());
  linkParameters.insert("box_id", gui->BoxIdLineEdit->text());

  Manager->linkProductionLineWithBoxManually(&linkParameters,
                                             ProductionLineModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_DeactivateAllProductionLinesPushButton_slot() {
  Logger->clear();

  Manager->shutdownAllProductionLinesManually(ProductionLineModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_UpdateProductionLineViewPushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("production_lines", ProductionLineModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_DeleteLastProductionLinePushButton_slot() {
  Logger->clear();

  Manager->deleteLastProductionLine(ProductionLineModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_ReleaseTransponderPushButton_slot()
{
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString> releaseParameters;
  QByteArray firmware;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkReleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные параметры для выпуска транспондера. ");
    return;
  }

  releaseParameters.insert("ucid", gui->UcidLineEdit->text());
  releaseParameters.insert("login", gui->LoginLineEdit2->text());
  releaseParameters.insert("password", gui->PasswordLineEdit2->text());
  // Сделал грязь
  releaseParameters.insert(
      "firmware_pointer",
      QString::number(reinterpret_cast<uint64_t>(&firmware)));

  Manager->releaseTransponderManually(&releaseParameters, TransponderSeed);

  // Отображение сгенерированной прошивки
  gui->AssembledFirmwareView->clear();
  gui->AssembledFirmwareView->appendPlainText(firmware.toHex());

  CurrentGUI->update();
}

void MainWindowKernel::on_ConfirmTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString> confirmParameters;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkReleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные параметры для подтверждения транспондера. ");
    return;
  }

  confirmParameters.insert("ucid", gui->UcidLineEdit->text());
  confirmParameters.insert("login", gui->LoginLineEdit2->text());
  confirmParameters.insert("password", gui->PasswordLineEdit2->text());

  Manager->confirmTransponderReleaseManually(&confirmParameters,
                                             TransponderSeed);

  CurrentGUI->update();
}

void MainWindowKernel::on_RereleaseTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString> rereleaseParameters;
  QString choice = gui->SearchTransponderByComboBox->currentText();
  QString input = gui->RereleaseTransponderLineEdit->text();
  QString ucid = gui->NewUcidLineEdit->text();
  QByteArray firmware;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkRereleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для перевыпуска транспондера. ");
    return;
  }

  rereleaseParameters.insert("login", gui->LoginLineEdit3->text());
  rereleaseParameters.insert("password", gui->PasswordLineEdit3->text());
  if (choice == "SN") {
    rereleaseParameters.insert("id", input);
  } else if (choice == "PAN") {
    rereleaseParameters.insert("personal_account_number", input);
  }
  rereleaseParameters.insert("ucid", ucid);
  // Сделал грязь
  rereleaseParameters.insert(
      "firmware_pointer",
      QString::number(reinterpret_cast<uint64_t>(&firmware)));

  Manager->rereleaseTransponderManually(&rereleaseParameters, TransponderSeed);

  // Отображение сгенерированной прошивки
  gui->AssembledFirmwareView->clear();
  gui->AssembledFirmwareView->appendPlainText(firmware.toHex());

  CurrentGUI->update();
}

void MainWindowKernel::on_ConfirmRereleaseTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString> confirmParameters;
  QString choice = gui->SearchTransponderByComboBox->currentText();
  QString input = gui->RereleaseTransponderLineEdit->text();
  QString ucid = gui->NewUcidLineEdit->text();

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkRereleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для подтверждения перевыпуска "
        "транспондера. ");
    return;
  }

  confirmParameters.insert("login", gui->LoginLineEdit3->text());
  confirmParameters.insert("password", gui->PasswordLineEdit3->text());
  if (choice == "SN") {
    confirmParameters.insert("id", input);
  } else if (choice == "PAN") {
    confirmParameters.insert("personal_account_number", input);
  }
  confirmParameters.insert("ucid", ucid);

  Manager->confirmTransponderRereleaseManually(&confirmParameters,
                                               TransponderSeed);

  CurrentGUI->update();
}

void MainWindowKernel::on_SearchTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString> searchParameters;
  QString text = gui->SearchTransponderByComboBox->currentText();

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkSearchTransponderInput()) {
    Interactor->generateError("Введены некорректные данные для поиска. ");
    return;
  }

  if (text == "UCID") {
    searchParameters.insert("ucid", gui->SearchTransponderLineEdit->text());
  } else if (text == "SN") {
    searchParameters.insert("id", gui->SearchTransponderLineEdit->text());
  } else if (text == "PAN") {
    searchParameters.insert("personal_account_number",
                            gui->SearchTransponderLineEdit->text());
  }

  Manager->searchTransponderManually(&searchParameters, TransponderSeed);

  CurrentGUI->update();
}

void MainWindowKernel::on_RefundTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString> refundParameters;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkSearchTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для возврата транспондера. ");
    return;
  }

  refundParameters.insert("ucid", gui->UcidLineEdit->text());
  refundParameters.insert("login", gui->LoginLineEdit2->text());
  refundParameters.insert("password", gui->PasswordLineEdit2->text());

  Manager->refundTransponderManually(&refundParameters, TransponderSeed);

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowIssuerTablePushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QString tableName = gui->IssuerTableChoice->currentText();
  Logger->clear();

  Manager->showDatabaseTable(MatchingTable->value(tableName), IssuerModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_InitTransportMasterKeysPushButton_slot() {
  Logger->clear();

  Manager->initTransportMasterKeys(IssuerModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_InitIssuerTablePushButton_slot() {
  Logger->clear();

  Manager->initIssuers(IssuerModel);

  CurrentGUI->update();
}

void MainWindowKernel::on_LinkIssuerWithKeysPushButton_slot() {
  QMap<QString, QString> linkParameters;
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QString issuerId = gui->IssuerIdLineEdit1->text();
  QString masterKeysId = gui->MasterKeysLineEdit1->text();
  QString masterKeysType = gui->MasterKeysChoice->currentText();

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkLinkIssuerInput()) {
    Interactor->generateError(
        "Введены некорректные данные для связывания эмитента с ключами. ");
    return;
  }

  // Собираем параметры
  linkParameters.insert("issuer_id", issuerId);
  linkParameters.insert("master_keys_id", masterKeysId);
  linkParameters.insert("master_keys_type",
                        MatchingTable->value(masterKeysType));

  Manager->linkIssuerWithMasterKeys(IssuerModel, &linkParameters);

  CurrentGUI->update();
}

void MainWindowKernel::on_ApplySettingsPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QSettings settings;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkNewSettings()) {
    Interactor->generateError("Введены некорректные данные для настроек. ");
    return;
  }

  // Считывание пользовательского ввода
  settings.setValue("PersoHost/Ip", gui->PersoServerIpLineEdit->text());
  settings.setValue("PersoHost/Port",
                    gui->PersoServerPortLineEdit->text().toInt());
  settings.setValue("PersoHost/MaxNumberClientConnection",
                    gui->MaxNumberClientConnectionLineEdit->text().toInt());
  settings.setValue("PersoHost/ClientConnection/MaxDuration",
                    gui->ClientConnectionMaxDurationLineEdit->text().toInt());
  settings.setValue("Database/Server/Ip", gui->DatabaseIpLineEdit->text());
  settings.setValue("Database/Server/Port",
                    gui->DatabasePortLineEdit->text().toInt());
  settings.setValue("Database/Name", gui->DatabaseNameLineEdit->text());
  settings.setValue("Database/User/Name",
                    gui->DatabaseUserNameLineEdit->text());
  settings.setValue("Database/User/Password",
                    gui->DatabaseUserPasswordLineEdit->text());
  settings.setValue("Database/Log/Active",
                    gui->DatabaseLogOption->checkState() == Qt::Checked);
  settings.setValue("Firmware/Base/Path",
                    gui->FirmwareBaseFilePathLineEdit->text());
  settings.setValue("Firmware/Data/Path",
                    gui->FirmwareDataFilePathLineEdit->text());

  // Применение новых настроек
  Manager->applySettings();

  // Оповещаем пользователя
  Interactor->generateNotification("Новые настройки успешно применены. ");
}

/*
 * Приватные методы
 */

void MainWindowKernel::proxyLogging(const QString& log) const {
  if (sender()->objectName() == QString("ServerManager"))
    emit logging(QString("Manager - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void MainWindowKernel::loadSettings() const {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);
}

bool MainWindowKernel::checkNewSettings() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  QHostAddress IP = QHostAddress(gui->PersoServerIpLineEdit->text());

  if (IP.isNull()) {
    return false;
  }

  int32_t port = gui->PersoServerPortLineEdit->text().toInt();

  if ((port > IP_PORT_MAX_VALUE) || (port < IP_PORT_MIN_VALUE)) {
    return false;
  }

  if (gui->MaxNumberClientConnectionLineEdit->text().toInt() <= 0) {
    return false;
  }

  if (gui->ClientConnectionMaxDurationLineEdit->text().toInt() <= 0) {
    return false;
  }

  IP = QHostAddress(gui->DatabaseIpLineEdit->text());

  if (IP.isNull()) {
    return false;
  }

  port = gui->DatabasePortLineEdit->text().toInt();

  if ((port > IP_PORT_MAX_VALUE) || (port < IP_PORT_MIN_VALUE)) {
    return false;
  }

  QFileInfo fileInfo(gui->FirmwareBaseFilePathLineEdit->text());
  if ((!fileInfo.isFile()) || (!fileInfo.exists())) {
    return false;
  }

  fileInfo.setFile(gui->FirmwareDataFilePathLineEdit->text());
  if ((!fileInfo.isFile()) || (!fileInfo.exists())) {
    return false;
  }

  return true;
}

bool MainWindowKernel::checkNewOrderInput() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  int32_t transponderQuantity =
      gui->TransponderQuantityLineEdit->text().toInt();
  int32_t boxCapacity = gui->BoxCapacityLineEdit->text().toInt();
  int32_t palletCapacity = gui->PalletCapacityLineEdit->text().toInt();
  QString transponderModel = gui->TransponderModelLineEdit->text();
  QString accrReference = gui->AccrReferenceLineEdit->text();
  QString equipmnetClass = gui->EquipmentClassLineEdit->text();
  QString manufacturerId = gui->ManufacturerIdLineEdit->text();
  QString panFilePath = gui->PanFilePathLineEdit->text();

  if (transponderQuantity <= 0) {
    return false;
  }

  if (boxCapacity <= 0) {
    return false;
  }

  if (palletCapacity <= 0) {
    return false;
  }

  if ((transponderQuantity % (boxCapacity * palletCapacity)) != 0) {
    return false;
  }

  if ((transponderModel.length() > TRANSPONDER_MODEL_CHAR_LENGTH) ||
      (transponderModel.length() == 0)) {
    return false;
  }

  QFileInfo info(gui->PanFilePathLineEdit->text());
  if ((!info.exists()) || (!info.isFile()) || (info.suffix() != "csv")) {
    return false;
  }

  QFile file(panFilePath);
  int32_t recordCount = 0;
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString record = in.readLine();
    if (record.length() != PAN_CHAR_LENGTH) {
      file.close();
      return false;
    }
    recordCount++;
  }
  file.close();

  if (recordCount != transponderQuantity) {
    return false;
  }

  if ((accrReference.length() > ACCR_REFERENCE_CHAR_LENGTH) ||
      (accrReference.length() == 0)) {
    return false;
  }

  if ((equipmnetClass.length() > EQUIPMENT_CLASS_CHAR_LENGTH) ||
      (equipmnetClass.length() == 0)) {
    return false;
  }

  if ((manufacturerId.length() > MANUFACTURER_ID_CHAR_LENGTH) ||
      (manufacturerId.length() == 0)) {
    return false;
  }

  return true;
}

bool MainWindowKernel::checkNewProductionLineInput() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QString login = gui->LoginLineEdit1->text();
  QString pass = gui->PasswordLineEdit1->text();

  if ((login.size() == 0) || (login.size() > 20)) {
    return false;
  }

  if ((pass.size() == 0) || (pass.size() > 20)) {
    return false;
  }

  return true;
}

bool MainWindowKernel::checkReleaseTransponderInput() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QRegularExpression regex("[A-Fa-f0-9]+");
  QString login = gui->LoginLineEdit2->text();
  QString pass = gui->PasswordLineEdit2->text();
  QString ucid = gui->UcidLineEdit->text();

  if (ucid.size() != UCID_CHAR_LENGTH) {
    return false;
  }

  QRegularExpressionMatch match = regex.match(ucid);
  if ((!match.hasMatch()) || (match.captured(0) != ucid)) {
    return false;
  }

  if ((login.size() == 0) || (login.size() > 20)) {
    return false;
  }

  if ((pass.size() == 0) || (pass.size() > 20)) {
    return false;
  }

  return true;
}

bool MainWindowKernel::checkSearchTransponderInput() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QRegularExpression ucidRegex("[A-Fa-f0-9]+");
  QRegularExpression panRegex("[0-9]+");
  QString choice = gui->SearchTransponderByComboBox->currentText();
  QString input = gui->SearchTransponderLineEdit->text();

  if (choice == "UCID") {
    if (input.size() != UCID_CHAR_LENGTH) {
      return false;
    }

    QRegularExpressionMatch match = ucidRegex.match(input);
    if ((!match.hasMatch()) || (match.captured(0) != input)) {
      return false;
    }
  } else if (choice == "SN") {
    if (input.toInt() == 0) {
      return false;
    }
  } else if (choice == "PAN") {
    if (input.length() != PAN_CHAR_LENGTH) {
      return false;
    }

    QRegularExpressionMatch match =
        panRegex.match(gui->SearchTransponderLineEdit->text());
    if ((!match.hasMatch()) || (match.captured(0) != input)) {
      return false;
    }
  } else {
    return false;
  }

  return true;
}

bool MainWindowKernel::checkRereleaseTransponderInput() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QRegularExpression ucidRegex("[A-Fa-f0-9]+");
  QRegularExpression panRegex("[0-9]+");
  QString choice = gui->RereleaseTransponderByComboBox->currentText();
  QString input = gui->RereleaseTransponderLineEdit->text();
  QString ucid = gui->NewUcidLineEdit->text();
  QString login = gui->LoginLineEdit3->text();
  QString pass = gui->PasswordLineEdit3->text();

  if ((login.size() == 0) || (login.size() > 20)) {
    return false;
  }

  if ((pass.size() == 0) || (pass.size() > 20)) {
    return false;
  }

  if (choice == "SN") {
    if (input.toInt() == 0) {
      return false;
    }
  } else if (choice == "PAN") {
    if (input.length() != PAN_CHAR_LENGTH) {
      return false;
    }

    QRegularExpressionMatch match = panRegex.match(input);
    if ((!match.hasMatch()) || (match.captured(0) != input)) {
      return false;
    }
  } else {
    return false;
  }

  if (ucid.size() != UCID_CHAR_LENGTH) {
    return false;
  }

  QRegularExpressionMatch match = ucidRegex.match(ucid);
  if ((!match.hasMatch()) || (match.captured(0) != ucid)) {
    return false;
  }

  return true;
}

bool MainWindowKernel::checkLinkIssuerInput() const {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QString issuerId = gui->IssuerIdLineEdit1->text();
  QString masterKeysId = gui->MasterKeysLineEdit1->text();

  if (issuerId.toInt() == 0) {
    return false;
  }

  if (masterKeysId.toInt() == 0) {
    return false;
  }

  return true;
}

void MainWindowKernel::createTopMenu() {
  menuBar()->clear();
  createTopMenuActions();

  ServiceMenu = menuBar()->addMenu("Сервис");
  ServiceMenu->addAction(OpenInitialGuiRequestAct);

  HelpMenu = menuBar()->addMenu("Справка");
  HelpMenu->addAction(AboutProgramAct);
}

void MainWindowKernel::createTopMenuActions() {
  OpenInitialGuiRequestAct = new QAction("Начальный интерфейс");
  OpenInitialGuiRequestAct->setStatusTip(
      "Закрыть текущий интерфейс и создать начальный интерфейс");
  connect(OpenInitialGuiRequestAct, &QAction::triggered, this,
          &MainWindowKernel::on_OpenInitialGuiRequestAct_slot);

  AboutProgramAct = new QAction("О программе", this);
  AboutProgramAct->setStatusTip("Показать сведения о программе");
}

void MainWindowKernel::createInitialInterface() {
  // Удаляем предыдущий интерфейс
  if (CurrentGUI) {
    CurrentGUI->hide();
    delete CurrentGUI;
  }

  // Создаем виджеты
  CurrentGUI = new InitialGUI(this);
  setCentralWidget(CurrentGUI);
  CurrentGUI->create();

  // Настраиваем размер главного окна
  setGeometry(DesktopGeometry.width() * 0.3, DesktopGeometry.height() * 0.3,
              DesktopGeometry.width() * 0.4, DesktopGeometry.height() * 0.4);
}

void MainWindowKernel::connectInitialInterface() {
  InitialGUI* gui = dynamic_cast<InitialGUI*>(CurrentGUI);

  connect(gui->OpenMasterGuiPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_OpenMasterGuiPushButton_slot);
  connect(gui->StartServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ServerStartPushButton_slot);
  connect(gui->StopServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ServerStopPushButton_slot);

  // Подключаем логгер
  connect(Logger, &LogSystem::requestDisplayLog, CurrentGUI, &GUI::displayLog);
  connect(Logger, &LogSystem::requestClearDisplayLog, CurrentGUI,
          &GUI::clearLogDisplay);
}

void MainWindowKernel::createMasterInterface() {
  // Удаляем предыдущий интерфейс
  if (CurrentGUI) {
    CurrentGUI->hide();
    delete CurrentGUI;
  }
  // Настраиваем размер главного окна
  setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1,
              DesktopGeometry.width() * 0.8, DesktopGeometry.height() * 0.8);

  // Создаем интерфейс
  CurrentGUI = new MasterGUI(this);
  setCentralWidget(CurrentGUI);
  CurrentGUI->create();

  // Создаем верхнее меню
  createTopMenu();
}

void MainWindowKernel::connectMasterInterface() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  // Сервер
  connect(gui->ServerStartPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ServerStartPushButton_slot);
  connect(gui->ServerStopPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ServerStopPushButton_slot);

  // База данных
  connect(gui->ConnectDatabasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ConnectDatabasePushButton_slot);
  connect(gui->DisconnectDatabasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_DisconnectDatabasePushButton_slot);
  connect(gui->ShowDatabaseTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowDatabaseTablePushButton_slot);
  connect(gui->ClearDatabaseTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ClearDatabaseTablePushButton_slot);
  connect(gui->InitIssuerTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_InitIssuerTablePushButton_slot);

  connect(gui->TransmitCustomRequestPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_TransmitCustomRequestPushButton_slot);

  // Заказы
  connect(gui->CreateNewOrderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_CreateNewOrderPushButton_slot);
  connect(gui->StartOrderAssemblingPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_StartOrderAssemblingPushButton_slot);
  connect(gui->StopOrderAssemblingPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_StopOrderAssemblingPushButton_slot);
  connect(gui->UpdateOrderViewPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_UpdateOrderViewPushButton_slot);
  connect(gui->DeleteLastOrderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_DeleteLastOrderPushButton_slot);

  // Производственные линии
  connect(gui->CreateNewProductionLinePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_CreateNewProductionLinePushButton_slot);
  connect(gui->AllocateInactiveProductionLinesPushButton, &QPushButton::clicked,
          this,
          &MainWindowKernel::on_AllocateInactiveProductionLinesPushButton_slot);
  connect(gui->LinkProductionLinePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_LinkProductionLinePushButton_slot);
  connect(gui->DeactivateAllProductionLinesPushButton, &QPushButton::clicked,
          this,
          &MainWindowKernel::on_DeactivateAllProductionLinesPushButton_slot);
  connect(gui->UpdateProductionLineViewPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_UpdateProductionLineViewPushButton_slot);
  connect(gui->DeleteLastProductionLinePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_DeleteLastProductionLinePushButton_slot);

  // Транспондеры
  connect(gui->ReleaseTransponderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ReleaseTransponderPushButton_slot);
  connect(gui->ConfirmTransponderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ConfirmTransponderPushButton_slot);
  connect(gui->RereleaseTransponderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_RereleaseTransponderPushButton_slot);
  connect(gui->ConfirmRereleaseTransponderPushButton, &QPushButton::clicked,
          this,
          &MainWindowKernel::on_ConfirmRereleaseTransponderPushButton_slot);
  connect(gui->SearchTransponderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_SearchTransponderPushButton_slot);
  connect(gui->RefundTransponderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_RefundTransponderPushButton_slot);

  // Эмитенты
  connect(gui->ShowIssuerTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowIssuerTablePushButton_slot);
  connect(gui->InitTransportMasterKeysPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_InitTransportMasterKeysPushButton_slot);
  connect(gui->LinkIssuerWithKeysPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_LinkIssuerWithKeysPushButton_slot);

  // Сохранение настроек
  connect(gui->ApplySettingsPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ApplySettingsPushButton_slot);

  // Подключаем логгер
  connect(Logger, &LogSystem::requestDisplayLog, CurrentGUI, &GUI::displayLog);
  connect(Logger, &LogSystem::requestClearDisplayLog, CurrentGUI,
          &GUI::clearLogDisplay);

  // Соединяем модели и представления
  gui->DatabaseRandomModelView->setModel(RandomModel);
  gui->OrderTableView->setModel(OrderModel);
  gui->ProductionLineTableView->setModel(ProductionLineModel);
  gui->TransponderSeedTableView->setModel(TransponderSeed);
  gui->IssuerTableView->setModel(IssuerModel);

  // Связываем отображения графиков с логикой их формирования
}
void MainWindowKernel::setupInterructionSystem() {
  delete Interactor;
  Interactor = new UserInteractionSystem(this, this);
}

void MainWindowKernel::setupManager(void) {
  delete Manager;
  Manager = new ServerManager(this);
  connect(Manager, &ServerManager::logging, this,
          &MainWindowKernel::proxyLogging);
  connect(Manager, &ServerManager::notifyUser, Interactor,
          &UserInteractionSystem::generateNotification);
  connect(Manager, &ServerManager::notifyUserAboutError, Interactor,
          &UserInteractionSystem::generateError);
  connect(Manager, &ServerManager::operationPerfomingStarted, Interactor,
          &UserInteractionSystem::generateProgressDialog);
  connect(Manager, &ServerManager::operationStepPerfomed, Interactor,
          &UserInteractionSystem::performeProgressDialogStep);
  connect(Manager, &ServerManager::operationPerformingFinished, Interactor,
          &UserInteractionSystem::completeProgressDialog);
}

void MainWindowKernel::setupLogger() {
  delete Logger;
  Logger = new LogSystem(this);
  connect(this, &MainWindowKernel::logging, Logger, &LogSystem::generate);
}

void MainWindowKernel::createModels() {
  RandomModel = new DatabaseTableModel(this);
  OrderModel = new DatabaseTableModel(this);
  ProductionLineModel = new DatabaseTableModel(this);
  IssuerModel = new DatabaseTableModel(this);
  TransponderSeed = new TransponderSeedModel(this);
}

void MainWindowKernel::createMatchingTable() {
  MatchingTable = new QMap<QString, QString>;
  MatchingTable->insert("Транспортные мастер ключи", "transport_master_keys");
  MatchingTable->insert("Коммерческие мастер ключи", "commercial_master_keys");
  MatchingTable->insert("Заказчики", "issuers");
}
