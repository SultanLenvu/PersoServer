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

  // Создаем графический интерфейс окна начальной конфигурации
  createInitialInterface();
  connectInitialInterface();

  // Cистема взаимодействия с пользователем
  setupInterructionSystem();

  // Управляющий модуль
  setupManager();

  // Создаем систему логгирования
  setupLogSystem();

  // Создаем модели для представлений
  createModels();
}

MainWindowKernel::~MainWindowKernel() {}

ServerManager* MainWindowKernel::manager() {
  return Manager;
}

void MainWindowKernel::openMasterInterface_slot() {
  createMasterInterface();
}

void MainWindowKernel::start_slot() {
  Logger->clear();
  Manager->start();
}

void MainWindowKernel::stop_slot() {
  Logger->clear();
  Manager->stop();
}

void MainWindowKernel::on_ConnectDatabasePushButton_slot() {
  Logger->clear();
}

void MainWindowKernel::on_DisconnectDatabasePushButton_slot() {
  Logger->clear();
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

void MainWindowKernel::on_InitIssuerTablePushButton_slot() {
  Logger->clear();

  Manager->initIssuers(RandomModel);

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

  orderParameters.insert("IssuerName", gui->IssuerNameComboBox->currentText());
  orderParameters.insert("TransponderQuantity",
                         gui->TransponderQuantityLineEdit->text());
  orderParameters.insert("BoxCapacity", gui->BoxCapacityLineEdit->text());
  orderParameters.insert("PalletCapacity", gui->PalletCapacityLineEdit->text());
  orderParameters.insert(
      "FullPersonalization",
      gui->FullPersonalizationCheckBox->checkState() == Qt::Checked ? "true"
                                                                    : "false");
  if (gui->FullPersonalizationCheckBox->checkState() == Qt::Checked) {
    orderParameters.insert("PanFilePath", gui->PanFilePathLineEdit->text());
  }
  orderParameters.insert("transponder_model",
                         gui->TransponderModelLineEdit->text());
  orderParameters.insert("accr_reference", gui->AccrReferenceLineEdit->text());

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
    Interactor->generateError("Некорректный ввод параметров нового заказа. ");
    return;
  }

  QMap<QString, QString> productionLineParameters;
  productionLineParameters.insert("Login", gui->LoginLineEdit1->text());
  productionLineParameters.insert("Password", gui->PasswordLineEdit1->text());
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
    Interactor->generateError("Некорректный ввод параметров нового заказа. ");
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
  QMap<QString, QString>* data = new QMap<QString, QString>;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkReleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для выпуска транспондера. ");
    delete data;
    return;
  }

  // Собираем сид транспондера
  data->insert("ucid", gui->UcidLineEdit->text());
  data->insert("login", gui->LoginLineEdit2->text());
  data->insert("password", gui->PasswordLineEdit2->text());
  TransponderSeed->build(data);

  // Осуществляем подтверждение выпуска
  Manager->releaseTransponderManually(TransponderSeed);

  // Обновляем интерфейс
  CurrentGUI->update();
}

void MainWindowKernel::on_ConfirmTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString>* data = new QMap<QString, QString>;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkReleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для подтверждения транспондера. ");
    delete data;
    return;
  }

  // Собираем сид транспондера
  data->insert("ucid", gui->UcidLineEdit->text());
  data->insert("login", gui->LoginLineEdit2->text());
  data->insert("password", gui->PasswordLineEdit2->text());
  TransponderSeed->build(data);

  // Осуществляем подтверждение выпуска
  Manager->confirmReleaseTransponderManually(TransponderSeed);

  // Обновляем интерфейс
  CurrentGUI->update();
}

void MainWindowKernel::on_RereleaseTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString>* data = new QMap<QString, QString>;
  QString choice = gui->SearchTransponderByComboBox->currentText();
  QString input = gui->RereleaseTransponderLineEdit->text();
  QString ucid = gui->NewUcidLineEdit->text();

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkRereleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для перевыпуска транспондера. ");
    delete data;
    return;
  }

  // Собираем сид транспондера
  data->insert("login", gui->LoginLineEdit3->text());
  data->insert("password", gui->PasswordLineEdit3->text());
  if (choice == "SN") {
    data->insert("id", input);
  } else if (choice == "PAN") {
    data->insert("payment_means", input);
  }
  data->insert("ucid", ucid);
  TransponderSeed->build(data);

  // Перевыпускаем транспондер
  Manager->rereleaseTransponderManually(TransponderSeed);

  // Обновляем интерфейс
  CurrentGUI->update();
}

void MainWindowKernel::on_ConfirmRereleaseTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString>* data = new QMap<QString, QString>;
  QString choice = gui->SearchTransponderByComboBox->currentText();
  QString input = gui->RereleaseTransponderLineEdit->text();
  QString ucid = gui->NewUcidLineEdit->text();

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkRereleaseTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для подтверждения перевыпуска "
        "транспондера. ");
    delete data;
    return;
  }

  // Собираем сид транспондера
  data->insert("login", gui->LoginLineEdit3->text());
  data->insert("password", gui->PasswordLineEdit3->text());
  if (choice == "SN") {
    data->insert("id", input);
  } else if (choice == "PAN") {
    data->insert("payment_means", input);
  }
  data->insert("ucid", ucid);
  TransponderSeed->build(data);

  // Осуществляем подтверждение перевыпуска
  Manager->confirmRereleaseTransponderManually(TransponderSeed);

  // Обновляем интерфейс
  CurrentGUI->update();
}

void MainWindowKernel::on_SearchTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString>* data = new QMap<QString, QString>;
  QString text = gui->SearchTransponderByComboBox->currentText();

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkSearchTransponderInput()) {
    Interactor->generateError("Введены некорректные данные для поиска. ");
    delete data;
    return;
  }

  // Собираем сид транспондера
  if (text == "UCID") {
    data->insert("ucid", gui->SearchTransponderLineEdit->text());
  } else if (text == "SN") {
    data->insert("id", gui->SearchTransponderLineEdit->text());
  } else if (text == "PAN") {
    data->insert("payment_means", gui->SearchTransponderLineEdit->text());
  }
  TransponderSeed->build(data);

  // Ищем информацию о транспондере
  Manager->searchTransponderManually(TransponderSeed);

  // Обновляем интерфейс
  CurrentGUI->update();
}

void MainWindowKernel::on_RefundTransponderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);
  QMap<QString, QString>* data = new QMap<QString, QString>;

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkSearchTransponderInput()) {
    Interactor->generateError(
        "Введены некорректные данные для возврата транспондера. ");
    delete data;
    return;
  }

  // Собираем сид транспондера
  data->insert("ucid", gui->UcidLineEdit->text());
  data->insert("login", gui->LoginLineEdit2->text());
  data->insert("password", gui->PasswordLineEdit2->text());
  TransponderSeed->build(data);

  // Осуществляем возврат
  Manager->refundTransponderManually(TransponderSeed);

  // Обновляем интерфейс
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
                    gui->FirmwareBasePathLineEdit->text());
  settings.setValue("Firmware/Data/Path",
                    gui->FirmwareDataPathLineEdit->text());

  // Применение новых настроек
  Manager->applySettings();

  // Оповещаем пользователя
  Interactor->generateNotification("Новые настройки успешно применены. ");
}

/*
 * Реализация слотов
 */

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

  IP = QHostAddress(gui->DatabaseIpLineEdit->text());

  if (IP.isNull()) {
    return false;
  }

  port = gui->DatabasePortLineEdit->text().toInt();

  if ((port > IP_PORT_MAX_VALUE) || (port < IP_PORT_MIN_VALUE)) {
    return false;
  }

  QFileInfo fileInfo(gui->FirmwareBasePathLineEdit->text());
  if ((!fileInfo.isFile()) || (fileInfo.suffix() != "hex") ||
      (!fileInfo.exists())) {
    return false;
  }

  fileInfo.setFile(gui->FirmwareDataPathLineEdit->text());
  if ((!fileInfo.isFile()) || (fileInfo.suffix() != "hex") ||
      (!fileInfo.exists())) {
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

  if (gui->FullPersonalizationCheckBox->checkState() == Qt::Checked) {
    QFileInfo info(gui->PanFilePathLineEdit->text());
    if ((!info.exists()) || (!info.isFile()) || (info.suffix() != "csv")) {
      return false;
    }
  }

  if ((transponderModel.length() > TRANSPONDER_MODEL_LENGTH) ||
      (transponderModel.length() == 0)) {
    return false;
  }

  if ((accrReference.length() > ACCR_REFERENCE_LENGTH) ||
      (accrReference.length() == 0)) {
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

  if (ucid.size() != UCID_LENGTH) {
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
    if (input.size() != UCID_LENGTH) {
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
    if (input.length() != PAYMENT_MEANS_LENGTH) {
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
    if (input.length() != PAYMENT_MEANS_LENGTH) {
      return false;
    }

    QRegularExpressionMatch match = panRegex.match(input);
    if ((!match.hasMatch()) || (match.captured(0) != input)) {
      return false;
    }
  } else {
    return false;
  }

  if (ucid.size() != UCID_LENGTH) {
    return false;
  }

  QRegularExpressionMatch match = ucidRegex.match(ucid);
  if ((!match.hasMatch()) || (match.captured(0) != ucid)) {
    return false;
  }

  return true;
}

void MainWindowKernel::createTopMenu() {
  menuBar()->clear();
  createTopMenuActions();

  ServiceMenu = menuBar()->addMenu("Сервис");
  ServiceMenu->clear();

  HelpMenu = menuBar()->addMenu("Справка");
  HelpMenu->addAction(AboutProgramAct);
}

void MainWindowKernel::createTopMenuActions() {
  AboutProgramAct = new QAction("О программе", this);
  AboutProgramAct->setStatusTip("Показать сведения о программе");
}

void MainWindowKernel::createInitialInterface() {
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

  connect(gui->OpenMasterPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::openMasterInterface_slot);
  connect(gui->StartServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::start_slot);
  connect(gui->StopServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::stop_slot);
}

void MainWindowKernel::createMasterInterface() {
  // Запрашиваем пароль
  //  QString key;
  //  Interactor->getMasterPassword(key);

  //  if (key == MASTER_ACCESS_PASSWORD) {
  // Удаляем предыдущий интерфейс
  CurrentGUI->hide();
  delete CurrentGUI;

  // Настраиваем размер главного окна
  setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1,
              DesktopGeometry.width() * 0.8, DesktopGeometry.height() * 0.8);

  // Создаем интерфейс
  CurrentGUI = new MasterGUI(this);
  setCentralWidget(CurrentGUI);
  CurrentGUI->create();

  // Создаем верхнее меню
  createTopMenu();

  // Соединяем графический интерфейс с ядром обработки
  connectMasterInterface();

  // Создаем систему логгирования
  setupLogSystem();
  //  } else {
  //    Interactor->generateError("Неверный код доступа");
  //  }
}

void MainWindowKernel::connectMasterInterface() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  // Меню взаимодействия с базой данных
  connect(gui->ConnectDatabasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ConnectDatabasePushButton_slot);
  connect(gui->DisconnectDatabasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_DisconnectDatabasePushButton_slot);

  // Базы данных
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

  // Сохранение настроек
  connect(gui->ApplySettingsPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ApplySettingsPushButton_slot);

  // Соединяем модели и представления
  gui->DatabaseRandomModelView->setModel(RandomModel);
  gui->OrderTableView->setModel(OrderModel);
  gui->ProductionLineTableView->setModel(ProductionLineModel);
  gui->TransponderSeedTableView->setModel(TransponderSeed);

  // Связываем отображения графиков с логикой их формирования
}
void MainWindowKernel::setupInterructionSystem() {
  Interactor = new UserInteractionSystem(this, this);
}

void MainWindowKernel::setupManager(void) {
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
  connect(Manager, &ServerManager::operationPerformingEnded, Interactor,
          &UserInteractionSystem::completeProgressDialog);
}

void MainWindowKernel::setupLogSystem() {
  delete Logger;
  Logger = new LogSystem(this);
  connect(this, &MainWindowKernel::logging, Logger, &LogSystem::generate);
  connect(Logger, &LogSystem::requestDisplayLog, CurrentGUI, &GUI::displayLog);
  connect(Logger, &LogSystem::requestClearDisplayLog, CurrentGUI,
          &GUI::clearLogDisplay);
}

void MainWindowKernel::createModels() {
  RandomModel = new DatabaseTableModel(this);
  OrderModel = new DatabaseTableModel(this);
  ProductionLineModel = new DatabaseTableModel(this);
  TransponderSeed = new TransponderInfoModel(this);
}
