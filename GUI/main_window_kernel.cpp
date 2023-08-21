#include "main_window_kernel.h"

MainWindowKernel::MainWindowKernel(QWidget* parent) : QMainWindow(parent) {
  // Считываем размеры дисплея
  DesktopGeometry = QApplication::desktop()->screenGeometry();

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

void MainWindowKernel::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("ServerManager"))
    emit logging(QString("Manager - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
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

void MainWindowKernel::on_ConnectDataBasePushButton_slot() {
  Logger->clear();
}

void MainWindowKernel::on_DisconnectDataBasePushButton_slot() {
  Logger->clear();
}

void MainWindowKernel::on_ShowProductionLineTablePushButton_slot() {
  Logger->clear();

  Manager->showProductionLines();

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowTransponderTablePushButton_slot() {
  Logger->clear();

  Manager->showTransponders();

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowOrderTablePushButton_slot() {
  Logger->clear();

  Manager->showOrders();

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowIssuerTablePushButton_slot() {
  Logger->clear();

  Manager->showIssuers();

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowBoxTablePushButton_slot() {
  Logger->clear();

  Manager->showBoxes();

  CurrentGUI->update();
}

void MainWindowKernel::on_ShowPalletPushButton_slot() {
  Logger->clear();

  Manager->showPallets();

  CurrentGUI->update();
}

void MainWindowKernel::on_TransmitCustomRequestPushButton_slot() {
  Logger->clear();

  Manager->showCustomResponse(
      dynamic_cast<GUI_Master*>(CurrentGUI)->CustomRequestLineEdit->text());

  CurrentGUI->update();
}

void MainWindowKernel::applyUserSettings_slot() {}

/*
 * Реализация слотов
 */

/*
 * Приватные методы
 */

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
  CurrentGUI = new GUI_Initial(this);
  setCentralWidget(CurrentGUI->create());
  // Настраиваем размер главного окна
  setGeometry(DesktopGeometry.width() * 0.3, DesktopGeometry.height() * 0.3,
              DesktopGeometry.width() * 0.4, DesktopGeometry.height() * 0.4);
}

void MainWindowKernel::connectInitialInterface() {
  GUI_Initial* gui = dynamic_cast<GUI_Initial*>(CurrentGUI);

  connect(gui->OpenMasterPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::openMasterInterface_slot);
  connect(gui->StartServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::start_slot);
  connect(gui->StopServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::stop_slot);
}

void MainWindowKernel::createMasterInterface() {
  QString key;
  emit requestMasterPassword(key);

  if (key == MASTER_MODE_ACCESS_KEY) {
    // Удаляем предыдущий интерфейс
    CurrentGUI->MainWidget->hide();
    delete CurrentGUI;

    // Настраиваем размер главного окна
    setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1,
                DesktopGeometry.width() * 0.8, DesktopGeometry.height() * 0.8);

    // Создаем интерфейс
    CurrentGUI = new GUI_Master(this);
    setCentralWidget(CurrentGUI->create());

    // Создаем верхнее меню
    createTopMenu();

    // Соединяем графический интерфейс с ядром обработки
    connectMasterInterface();

    // Создаем систему логгирования
    setupLogSystem();
  } else
    emit notifyUserAboutError("Неверный код доступа");
}

void MainWindowKernel::connectMasterInterface() {
  GUI_Master* gui = dynamic_cast<GUI_Master*>(CurrentGUI);

  // Меню взаимодействия с базой данных
  connect(gui->ConnectDataBasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ConnectDataBasePushButton_slot);
  connect(gui->DisconnectDataBasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_DisconnectDataBasePushButton_slot);

  connect(gui->ShowProductionLineTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowProductionLineTablePushButton_slot);
  connect(gui->ShowTransponderTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowTransponderTablePushButton_slot);
  connect(gui->ShowOrderTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowOrderTablePushButton_slot);
  connect(gui->ShowIssuerTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowIssuerTablePushButton_slot);
  connect(gui->ShowBoxTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowBoxTablePushButton_slot);
  connect(gui->ShowPalletPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_ShowPalletPushButton_slot);

  connect(gui->TransmitCustomRequestPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::on_TransmitCustomRequestPushButton_slot);

  // Сохранение настроек
  connect(gui->ApplySettingsPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::applyUserSettings_slot);

  // Соединяем модели и представления
  gui->DataBaseBufferView->setModel(Manager->buffer());

  // Связываем отображения графиков с логикой их формирования
}
void MainWindowKernel::setupInterructionSystem() {
  InteractionSystem = new UserInteractionSystem(this, this);
  connect(this, &MainWindowKernel::notifyUser, InteractionSystem,
          &UserInteractionSystem::generateNotification);
  connect(this, &MainWindowKernel::requestMasterPassword, InteractionSystem,
          &UserInteractionSystem::getMasterPassword);
  connect(this, &MainWindowKernel::notifyUserAboutError, InteractionSystem,
          &UserInteractionSystem::generateError);
}

void MainWindowKernel::setupManager(void) {
  Manager = new ServerManager(this);
  connect(Manager, &ServerManager::logging, this,
          &MainWindowKernel::proxyLogging);
  connect(Manager, &ServerManager::notifyUser, InteractionSystem,
          &UserInteractionSystem::generateNotification);
  connect(Manager, &ServerManager::notifyUserAboutError, InteractionSystem,
          &UserInteractionSystem::generateError);
}

void MainWindowKernel::setupLogSystem() {
  delete Logger;
  Logger = new LogSystem(this);
  connect(this, &MainWindowKernel::logging, Logger, &LogSystem::generate);
  connect(Logger, &LogSystem::requestDisplayLog, CurrentGUI, &GUI::displayLog);
  connect(Logger, &LogSystem::requestClearDisplayLog, CurrentGUI,
          &GUI::clearLogDisplay);
}
