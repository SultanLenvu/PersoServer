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

PersoManager* MainWindowKernel::manager() {
  return Manager;
}

void MainWindowKernel::proxyLogging(const QString& log) {
  if (sender()->objectName() == QString("PersoManager"))
    emit logging(QString("Manager - ") + log);
  else
    emit logging(QString("Unknown - ") + log);
}

void MainWindowKernel::openMasterInterface_slot() {
  createMasterInterface();
}

void MainWindowKernel::startServer_slot() {
  CurrentGUI->clearLogDisplay();
  Manager->startServer();
}

void MainWindowKernel::stopServer_slot() {
  CurrentGUI->clearLogDisplay();
  Manager->stopServer();
}

void MainWindowKernel::connectDataBase_slot() {
  CurrentGUI->clearLogDisplay();

  Manager->connectDatabase();
}

void MainWindowKernel::disconnectDataBase_slot() {
  CurrentGUI->clearLogDisplay();

  Manager->disconnectDatabase();
}

void MainWindowKernel::transamitCustomRequest_slot() {
  CurrentGUI->clearLogDisplay();

  Manager->performCustomSqlRequest(
      dynamic_cast<GUI_Master*>(CurrentGUI)->CustomRequestLineEdit->text());

  //  dynamic_cast<GUI_Master*>(CurrentGUI)->DataBaseBufferView->setModel(nullptr);
  //  dynamic_cast<GUI_Master*>(CurrentGUI)
  //      ->DataBaseBufferView->setModel(Manager->buffer());
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
          &MainWindowKernel::startServer_slot);
  connect(gui->StopServerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::stopServer_slot);
}

void MainWindowKernel::createMasterInterface() {
  QString key;
  emit requestUserInputKey(key);

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
          &MainWindowKernel::connectDataBase_slot);
  connect(gui->DisconnectDataBasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::disconnectDataBase_slot);
  connect(gui->TransmitCustomRequestPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::transamitCustomRequest_slot);

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
  connect(this, &MainWindowKernel::requestUserInputKey, InteractionSystem,
          &UserInteractionSystem::getUserInputKey);
  connect(this, &MainWindowKernel::notifyUserAboutError, InteractionSystem,
          &UserInteractionSystem::generateError);
}

void MainWindowKernel::setupManager(void) {
  Manager = new PersoManager(this);
  connect(Manager, &PersoManager::logging, this,
          &MainWindowKernel::proxyLogging);
  connect(Manager, &PersoManager::notifyUser, InteractionSystem,
          &UserInteractionSystem::generateNotification);
  connect(Manager, &PersoManager::notifyUserAboutError, InteractionSystem,
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