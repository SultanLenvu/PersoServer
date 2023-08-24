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

void MainWindowKernel::on_ShowProductionLineTablePushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("production_lines");

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowTransponderTablePushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("transponders");

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowOrderTablePushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("orders");

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowIssuerTablePushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("issuers");

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowBoxTablePushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("boxes");

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowPalletPushButton_slot() {
  Logger->clear();

  Manager->showDatabaseTable("pallets");

  CurrentGUI->update();
}

void MainWindowKernel::on_TransmitCustomRequestPushButton_slot() {
  Logger->clear();

  Manager->showCustomResponse(
      dynamic_cast<MasterGUI*>(CurrentGUI)->CustomRequestLineEdit->text());

  CurrentGUI->update();
}

void MainWindowKernel::on_CreateNewOrderPushButton_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  Logger->clear();

  if (!checkNewOrderInput()) {
    Interactor->generateError("Некорректный ввод параметров нового заказа. ");
    return;
  }

  IssuerOrder newOrder(nullptr);

  newOrder.setIssuerName(gui->IssuerNameComboBox->currentText());
  newOrder.setTransponderQuantity(
      gui->TransponderQuantityLineEdit->text().toInt());

  if (gui->FullPersonalizationCheckBox->checkState() == Qt::Checked) {
    newOrder.setFullPersonalization(gui->PanFilePathLineEdit->text());
  }

  Manager->createNewOrder(&newOrder);

  CurrentGUI->update();
}

void MainWindowKernel::applyUserSettings_slot() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  Logger->clear();

  // Проверка пользовательского ввода
  if (!checkNewSettings()) {
    Interactor->generateError("Введены некорректные данные для настроек. ");
    return;
  }

  // Считывание пользовательского ввода
  Settings->setValue("PersoHost/Ip", gui->PersoServerPortLineEdit->text());
  Settings->setValue("PersoHost/Port",
                     gui->PersoServerPortLineEdit->text().toInt());
  Settings->setValue("Database/Server/Ip", gui->DatabaseIpLineEdit->text());
  Settings->setValue("Database/Server/Port",
                     gui->DatabasePortLineEdit->text().toInt());
  Settings->setValue("Database/Name", gui->DatabaseNameLineEdit->text());
  Settings->setValue("Database/User/Name",
                     gui->DatabaseUserNameLineEdit->text());
  Settings->setValue("Database/User/Password",
                     gui->DatabaseUserPasswordLineEdit->text());

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

void MainWindowKernel::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("ServerManager"))
    emit logging(QString("Manager - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void MainWindowKernel::loadSettings() {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);

  Settings = new QSettings(this);
}

bool MainWindowKernel::checkNewSettings() {
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

  return true;
}

bool MainWindowKernel::checkNewOrderInput() {
  MasterGUI* gui = dynamic_cast<MasterGUI*>(CurrentGUI);

  if (gui->TransponderQuantityLineEdit->text().toInt() == 0) {
    return false;
  }

  if (gui->FullPersonalizationCheckBox->checkState() == Qt::Checked) {
    QFileInfo info(gui->PanFilePathLineEdit->text());
    if ((!info.exists()) || (!info.isFile()) || (info.suffix() != "csv")) {
      return false;
    }
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
  connect(gui->TransmitCustomRequestPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_TransmitCustomRequestPushButton_slot);

  // Производственные линии
  connect(gui->UpdateProductionLinePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowProductionLineTablePushButton_slot);

  // Транспондеры
  connect(gui->UpdateTransponderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowTransponderTablePushButton_slot);

  // Заказы
  connect(gui->UpdateOrderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowOrderTablePushButton_slot);

  // Эмитенты
  connect(gui->UpdateIssuerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowIssuerTablePushButton_slot);

  // Боксы
  connect(gui->UpdateBoxPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowBoxTablePushButton_slot);

  // Палеты
  connect(gui->UpdatePalletPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowPalletPushButton_slot);

  // Создание нового заказа
  connect(gui->CreateNewOrderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_CreateNewOrderPushButton_slot);

  // Сохранение настроек
  connect(gui->ApplySettingsPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::applyUserSettings_slot);

  // Соединяем модели и представления
  // gui->DatabaseBufferView->setModel(Manager->buffer());

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
