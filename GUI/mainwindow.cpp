#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
  GUI = new MainWindow_GUI(this, this);
  GUI->createInterface(MainWindow_GUI::Initial);
  connectInitialInterface();

  CurrentDtr = nullptr;
  Manager = nullptr;

  LogSystem = new GlobalLogSystem(this);
  connect(LogSystem, &GlobalLogSystem::displayLogging, this, &MainWindow::displayLogData);

  // Cистема оповещения пользователя
  InteractionSystem = new UserInteractionSystem(this, this);
  connect(this, &MainWindow::notifyUser, InteractionSystem, &UserInteractionSystem::generateNotification);
  connect(this, &MainWindow::requestUserInputKey, InteractionSystem, &UserInteractionSystem::getUserInputKey);
  connect(this, &MainWindow::notifyUserAboutError, InteractionSystem, &UserInteractionSystem::generateError);
}

MainWindow::~MainWindow()
{
}

/*
 * Реализация слотов
 */

void MainWindow::displayLogData(QString* log)
{
  if (GUI->CurrentMode == MainWindow_GUI::Master)
  {
    if (GUI->GeneralLogs->toPlainText().length() > 200000)
      GUI->GeneralLogs->clear();

    if (GUI->GeneralLogs)
      GUI->GeneralLogs->appendPlainText(*log);
  }
}

void MainWindow::connectMD5859_slot()
{
  CurrentDtr = new MD5859(this);
  connect(CurrentDtr, &DTR::logging, LogSystem, &GlobalLogSystem::dtrLogging);

  processingDtrConnection();
}

void MainWindow::connectTC278_slot()
{
  CurrentDtr = new TC278(this);
  connect(CurrentDtr, &DTR::logging, LogSystem, &GlobalLogSystem::dtrLogging);

  processingDtrConnection();
}

void MainWindow::connect_slot()
{
  CurrentDtr->connectInterface();
}

void MainWindow::disconnect_slot()
{
  CurrentDtr->disconnectInterface();

  delete CurrentDtr;
  delete Manager;
  CurrentDtr = nullptr;
  Manager = nullptr;

  GUI->destroyInterface();
  GUI->createInterface(MainWindow_GUI::Initial);
  connectInitialInterface();
}

void MainWindow::reboot_slot()
{
  GUI->GeneralLogs->clear();
  CurrentDtr->reboot();
}

void MainWindow::status_slot()
{
  GUI->GeneralLogs->clear();
  CurrentDtr->status();
}

void MainWindow::transmitCustomData_slot()
{
  QString str = GUI->LineEditCustomData->text();
  CurrentDtr->transmitCustomData(&str);
}

void MainWindow::enableCommunication_slot()
{
  dynamic_cast<MD5859*>(CurrentDtr)->enableCommunication();
}

void MainWindow::disableCommunication_slot()
{
  dynamic_cast<MD5859*>(CurrentDtr)->disableCommunication();
}

void MainWindow::configure_slot()
{
  dynamic_cast<MD5859*>(CurrentDtr)->configure();
}

void MainWindow::cardme4Transaction_slot()
{
  GUI->GeneralLogs->clear();

  Manager->processing(DsrcManager::CARDME4, CurrentDtr);

  GUI->updateObuDataViews();
}

void MainWindow::read_slot()
{
  GUI->GeneralLogs->clear();

  Manager->processing(DsrcManager::Read, CurrentDtr);

  GUI->updateObuDataViews();
}

void MainWindow::clear_slot()
{
  GUI->GeneralLogs->clear();

  Manager->processing(DsrcManager::Clear, CurrentDtr);

  GUI->updateObuDataViews();
}

void MainWindow::personalize_slot()
{
  GUI->GeneralLogs->clear();

  Manager->processing(DsrcManager::Personalization, CurrentDtr);

  GUI->updateObuDataViews();
}

void MainWindow::hardReset_slot()
{
  GUI->GeneralLogs->clear();

  Manager->processing(DsrcManager::ResetHard, CurrentDtr);

  GUI->updateObuDataViews();
}

void MainWindow::overwriteAttribute_slot()
{
  GUI->GeneralLogs->clear();

  DsrcElement::ElementId eid = static_cast<DsrcElement::ElementId>(GUI->ElementChoice->currentIndex());
  DsrcAttribute* attribute = nullptr;

  if (eid == DsrcElement::System)
    attribute = new DsrcAttribute(static_cast<SystemAttributeName>(GUI->AttributeChoice->currentIndex()));
  else
    attribute = new DsrcAttribute(static_cast<EfcAttributeName>(GUI->AttributeChoice->currentIndex()));

  QString newValue = GUI->AttributeNewValue->text();
  attribute->setValue(&newValue);

  Manager->overwriteAttribute(CurrentDtr, eid, attribute);

  GUI->updateObuDataViews();
}

void MainWindow::fullTesting_slot()
{
  GUI->GeneralLogs->clear();
  Manager->conductObuFullTesting(CurrentDtr);

  GUI->updateTestTableViews();
}

void MainWindow::singleTest_slot()
{
  GUI->GeneralLogs->clear();
  Manager->conductObuSingleTest(CurrentDtr, GUI->LineEditTestName->text());

  GUI->updateTestTableViews();
}

void MainWindow::appIKTest_slot()
{
  GUI->GeneralLogs->clear();
  Manager->conductObuClassTest(CurrentDtr, QString("TP/AP/IK/"));

  GUI->updateTestTableViews();
}

void MainWindow::appTKTest_slot()
{
  GUI->GeneralLogs->clear();
  Manager->conductObuClassTest(CurrentDtr, QString("TP/AP/TK/"));

  GUI->updateTestTableViews();
}

void MainWindow::efcTest_slot()
{
  GUI->GeneralLogs->clear();
  Manager->conductObuClassTest(CurrentDtr, QString("TP/EFC/"));

  GUI->updateTestTableViews();
}

void MainWindow::systemTest_slot()
{
  GUI->GeneralLogs->clear();
  Manager->conductObuClassTest(CurrentDtr, QString("TP/System/"));

  GUI->updateTestTableViews();
}

void MainWindow::startEndlessMeasuring_slot()
{
}

void MainWindow::stopEndlessMeasuring_slot()
{
}

void MainWindow::measureCountable_slot()
{
  GUI->GeneralLogs->clear();

  bool ok = false;
  uint32_t measurementCount = GUI->LineEditMeasureCount->text().toInt(&ok, 10);

  if (ok == true)
    Manager->conductEnergyConsumptionTest(CurrentDtr, measurementCount);
  else
    notifyUser(QString("Введено некорректное значение количества замеров. "));
}

void MainWindow::saveSettings_slot()
{
  Manager->settings()->setMasterKeyFilePath(GUI->localMasterKeyPathLineEdit->text());
  Manager->settings()->setPerosnalizationServerIP(GUI->ipAddressPersoServerLineEdit->text());
  Manager->settings()->setServerCommonKeyGeneration(GUI->ServerCommonKeyGenerationCheckBox->isChecked());
  Manager->settings()->setServerPersonalizationOption(GUI->UseServerPersoCheckBox->isChecked());
}

void MainWindow::elementChoice_slot(int index)
{
  GUI->AttributeChoice->clear();
  for (int32_t i = 0; i < Manager->dsrcCore()->currentOBU()->elements()->at(index)->attributeCount(); i++)
    GUI->AttributeChoice->addItem(
        Manager->dsrcCore()->currentOBU()->elements()->at(index)->attributes()->at(i)->name());
}

void MainWindow::productionInterfaceRequest_slot()
{
  setupDsrcManager();
  GUI->destroyInterface();
  GUI->createInterface(MainWindow_GUI::Production);
  connectProductionInterface();
}

void MainWindow::masterInterfaceRequest_slot()
{
  setupDsrcManager();
  GUI->destroyInterface();
  GUI->createInterface(MainWindow_GUI::Master);
  connectMasterInterface();
}

void MainWindow::connectInitialInterface()
{
  // Соединяем сигналы и слоты
  connect(GUI->PushButtonConnectMD5859, &QPushButton::clicked, this, &MainWindow::connectMD5859_slot);
  connect(GUI->PushButtonConnectTC278, &QPushButton::clicked, this, &MainWindow::connectTC278_slot);
}

void MainWindow::connectProductionInterface()
{
  // Соединяем сигналы и слоты
  connect(GUI->PI_PushButtonPersonalize, &QPushButton::clicked, this, &MainWindow::personalize_slot);
  connect(GUI->PI_PushButtonHardReset, &QPushButton::clicked, this, &MainWindow::hardReset_slot);
  connect(GUI->MasterInterfaceRequestAct, &QAction::triggered, this, &MainWindow::masterInterfaceRequest_slot);

  // Соединяем модели и представления
  GUI->PI_MI_SystemElementView->setModel(Manager->dsrcCore()->currentOBU()->efc1Element());
  GUI->PI_EfcElementView->setModel(Manager->dsrcCore()->currentOBU()->efc1Element());
}

void MainWindow::connectMasterInterface()
{
  // Соединяем сигналы и слоты
  connect(GUI->PushButtonConnect, &QPushButton::clicked, this, &MainWindow::connect_slot);
  connect(GUI->PushButtonDisconnect, &QPushButton::clicked, this, &MainWindow::disconnect_slot);

  connect(GUI->PushButtonReboot, &QPushButton::clicked, this, &MainWindow::reboot_slot);
  connect(GUI->PushButtonStatus, &QPushButton::clicked, this, &MainWindow::status_slot);

  connect(GUI->PushButtonTransmitCustomData, &QPushButton::clicked, this, &MainWindow::transmitCustomData_slot);

  if (GUI->CurrentDtr == MainWindow_GUI::TC278)
  {
    //  connect(GUI->PushButtonEnableSynchronization, &QPushButton::clicked, this,
    //          &MainWindow::enableSynchronization_slot);
    //  connect(GUI->PushButtonDisableSynchronization, &QPushButton::clicked,
    //  this,
    //          &MainWindow::disableSynchronization_slot);
    //  connect(GUI->PushButtonSetAttenuation, &QPushButton::clicked, this,
    //          &MainWindow::setAttenuation_slot);
    //  connect(GUI->PushButtonGetAttenuation, &QPushButton::clicked, this,
    //          &MainWindow::getAttenuation_slot);
  }
  else
  {
    connect(GUI->PushButtonEnableCommunication, &QPushButton::clicked, this, &MainWindow::enableCommunication_slot);
    connect(GUI->PushButtonDisableCommunication, &QPushButton::clicked, this, &MainWindow::disableCommunication_slot);
    connect(GUI->PushButtonConfigure, &QPushButton::clicked, this, &MainWindow::configure_slot);
  }

  connect(GUI->PushButtonRead, &QPushButton::clicked, this, &MainWindow::read_slot);
  connect(GUI->MI_PushButtonPersonalize, &QPushButton::clicked, this, &MainWindow::personalize_slot);
  connect(GUI->PushButtonClear, &QPushButton::clicked, this, &MainWindow::clear_slot);
  connect(GUI->MI_PushButtonHardReset, &QPushButton::clicked, this, &MainWindow::hardReset_slot);
  connect(GUI->ElementChoice, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::elementChoice_slot);
  connect(GUI->PushButtonOverwriteAttribute, &QPushButton::clicked, this, &MainWindow::overwriteAttribute_slot);

  connect(GUI->PushButtonFullTesting, &QPushButton::clicked, this, &MainWindow::fullTesting_slot);
  connect(GUI->PushButtonAppIKTest, &QPushButton::clicked, this, &MainWindow::appIKTest_slot);
  connect(GUI->PushButtonAppTKTest, &QPushButton::clicked, this, &MainWindow::appTKTest_slot);
  connect(GUI->PushButtonEfcTest, &QPushButton::clicked, this, &MainWindow::efcTest_slot);
  connect(GUI->PushButtonSystemTest, &QPushButton::clicked, this, &MainWindow::systemTest_slot);
  connect(GUI->PushButtonSingleTest, &QPushButton::clicked, this, &MainWindow::singleTest_slot);

  connect(GUI->PushButtonStartEndlessMeasuring, &QPushButton::clicked, this, &MainWindow::startEndlessMeasuring_slot);
  connect(GUI->PushButtonStopEndlessMeasuring, &QPushButton::clicked, this, &MainWindow::stopEndlessMeasuring_slot);
  connect(GUI->PushButtonMeasureCountableTimes, &QPushButton::clicked, this, &MainWindow::measureCountable_slot);

  connect(GUI->PushButtonSaveSettings, &QPushButton::clicked, this, &MainWindow::saveSettings_slot);

  connect(GUI->ProductionInterfaceRequestAct, &QAction::triggered, this, &MainWindow::productionInterfaceRequest_slot);

  // Соединяем модели и представления
  GUI->DtrParametersView->setModel(CurrentDtr);
  GUI->MI_EfcElementView->setModel(Manager->dsrcCore()->currentOBU()->efc1Element());
  GUI->MI_SystemElementView->setModel(Manager->dsrcCore()->currentOBU()->systemElement());

  GUI->CommonKeysView->setModel(Manager->dsrcCore()->currentOBU()->keys());
  GUI->TMasterKeysView->setModel(Manager->dsrcCore()->applicationLayer()->sKernel()->transportMasterKeys());
  GUI->CMasterKeysView->setModel(Manager->dsrcCore()->applicationLayer()->sKernel()->commercialMasterKeys());

  GUI->AppIKTestsView->setModel(Manager->tester()->ikTests());
  GUI->AppTKTestsView->setModel(Manager->tester()->tkTests());
  GUI->EfcTestsView->setModel(Manager->tester()->efcTests());
  GUI->SystemTestsView->setModel(Manager->tester()->systemTests());

  // Связываем отображения графиков с логикой их формирования
  Manager->tester()->setBatteryVoltageChartView(GUI->BatteryVoltageChartView);

  GUI->ElementChoice->addItem(QString("System"));
  GUI->ElementChoice->addItem(QString("EFC1"));
}

/*
 * Приватные методы
 */

void MainWindow::processingDtrConnection(void)
{
  QString key;

  CurrentDtr->connectInterface();

  if (CurrentDtr->isConnected())
  {
    emit requestUserInputKey(key);

    if (key == MASTER_MODE_ACCESS_KEY)
    {
      setupDsrcManager();

      GUI->destroyInterface();
      GUI->createInterface(MainWindow_GUI::Master);

      connectMasterInterface();
    }
    else if (key == PRODUCTION_MODE_ACCESS_KEY)
    {
      setupDsrcManager();

      GUI->destroyInterface();
      GUI->createInterface(MainWindow_GUI::Production);

      connectProductionInterface();
    }
  }
  else
    emit notifyUserAboutError(QString("Соединение не может быть установлено. "));
}

void MainWindow::setupDsrcManager(void)
{
  delete Manager;
  Manager = new DsrcManager(this, LogSystem);
  connect(Manager, &DsrcManager::notifyUser, InteractionSystem, &UserInteractionSystem::generateNotification);
  connect(Manager, &DsrcManager::notifyUserAboutError, InteractionSystem, &UserInteractionSystem::generateError);
}
