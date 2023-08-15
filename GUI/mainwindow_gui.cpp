#include "mainwindow_gui.h"

// Первичное отображение главного окна
MainWindow_GUI::MainWindow_GUI(QObject* parent, QMainWindow* mainWindow) : QObject(parent)
{
  MainWindow = mainWindow;

  MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
  MainWindow->setLayoutDirection(Qt::LeftToRight);
  DesktopGeometry = QApplication::desktop()->screenGeometry();

  CurrentDtr = TC278;
  CurrentMode = Initial;
}

MainWindow_GUI::~MainWindow_GUI()
{
}

void MainWindow_GUI::setCurrentDtr(DtrType dtr)
{
  CurrentDtr = dtr;
}

void MainWindow_GUI::createInterface(OperatingMode mode)
{
  CurrentMode = mode;

  CurrentInterface = new QWidget();
  MainWindow->setCentralWidget(CurrentInterface);

  // Настраиваем размер главного окна
  switch (CurrentMode)
  {
    case Initial:
      createInitialInterface();
      break;
    case Production:
      createProductionInterface();
      break;
    case Master:
      createMasterInterface();
      break;
  }

  // Подключаем неявное связывание сигналов и слотов по их имени
  QMetaObject::connectSlotsByName(MainWindow);
}

void MainWindow_GUI::destroyInterface()
{
  QObject::disconnect(CurrentInterface);
  delete CurrentInterface;
  CurrentInterface = nullptr;
}

void MainWindow_GUI::updateTestTableViews()
{
  AppIKTestsView->resizeColumnsToContents();
  AppTKTestsView->resizeColumnsToContents();
  EfcTestsView->resizeColumnsToContents();
  SystemTestsView->resizeColumnsToContents();
}

void MainWindow_GUI::updateObuDataViews()
{
  MI_EfcElementView->resizeColumnsToContents();
  MI_SystemElementView->resizeColumnsToContents();
}

void MainWindow_GUI::updateObuKeysViews()
{
  CommonKeysView->resizeColumnsToContents();
  TMasterKeysView->resizeColumnsToContents();
  CMasterKeysView->resizeColumnsToContents();
}

/*
 * Приватные методы
 */

void MainWindow_GUI::createInitialInterface()
{
  // Настраиваем размер главного окна
  MainWindow->setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1, DesktopGeometry.width() * 0.4,
                          DesktopGeometry.height() * 0.2);

  II_MainLayout = new QHBoxLayout();
  CurrentInterface->setLayout(II_MainLayout);

  // Создаем кнопки для подключения к десктоп ридерам
  create_II_Buttons();
}

void MainWindow_GUI::createProductionInterface()
{
  // Настраиваем размер главного окна
  MainWindow->setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1, DesktopGeometry.width() * 0.8,
                          DesktopGeometry.height() * 0.8);

  PI_MainLayout = new QHBoxLayout();
  CurrentInterface->setLayout(PI_MainLayout);

  // Создаем кнопки
  create_PI_Buttons();

  // Создаем отображения данных транспондера
  create_PI_ObuDataView();

  // Настраиваем пропорции отображаемых элементов
  PI_MainLayout->setStretch(0, 1);
  PI_MainLayout->setStretch(1, 3);

  // Создаем верхнее меню
  createTopMenu();
}

void MainWindow_GUI::createMasterInterface()
{
  // Настраиваем размер главного окна
  MainWindow->setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1, DesktopGeometry.width() * 0.8,
                          DesktopGeometry.height() * 0.8);

  MI_MainLayout = new QHBoxLayout();
  CurrentInterface->setLayout(MI_MainLayout);

  // Создаем вкладки мастер меню
  create_MI_Tabs();

  // Создаем виджеты логов
  create_MI_LogWidgets();

  // Настраиваем пропорции отображаемых элементов
  MI_MainLayout->setStretch(0, 2);
  MI_MainLayout->setStretch(1, 1);

  // Создаем верхнее меню
  createTopMenu();
}

void MainWindow_GUI::createTopMenu(void)
{
  MainWindow->menuBar()->clear();
  createTopMenuActions();

  ServiceMenu = MainWindow->menuBar()->addMenu("Сервис");
  ServiceMenu->clear();
  // ServiceMenu->addSeparator();
  if (CurrentMode == Production)
    ServiceMenu->addAction(MasterInterfaceRequestAct);
  else
    ServiceMenu->addAction(ProductionInterfaceRequestAct);

  HelpMenu = MainWindow->menuBar()->addMenu("Справка");
  HelpMenu->addAction(AboutProgramAct);
  HelpMenu->addAction(AboutProgramAct);
}

void MainWindow_GUI::createTopMenuActions(void)
{
  if (CurrentMode == Master)
  {
    ProductionInterfaceRequestAct = new QAction("Производственный режим", MainWindow);
    ProductionInterfaceRequestAct->setStatusTip("Перейти в производственный режим");
  }
  else
  {
    MasterInterfaceRequestAct = new QAction("Мастер доступ", MainWindow);
    MasterInterfaceRequestAct->setStatusTip("Перейти в мастер доступ");
  }

  AboutProgramAct = new QAction("О программе", MainWindow);
  AboutProgramAct->setStatusTip("Показать сведения о программе");
}

void MainWindow_GUI::create_II_Buttons()
{
  II_ConnectButtonGroup = new QGroupBox("Выбор DTR");
  II_ConnectButtonGroup->setAlignment(Qt::AlignCenter);
  II_MainLayout->addWidget(II_ConnectButtonGroup);

  II_ConnectButtonLayout = new QVBoxLayout();
  II_ConnectButtonGroup->setLayout(II_ConnectButtonLayout);

  PushButtonConnectMD5859 = new QPushButton("Подключиться к MD5859");
  II_ConnectButtonLayout->addWidget(PushButtonConnectMD5859);
  PushButtonConnectTC278 = new QPushButton("Подключиться к TC289");
  II_ConnectButtonLayout->addWidget(PushButtonConnectTC278);

  II_ConnectButtonSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  II_ConnectButtonLayout->addItem(II_ConnectButtonSpacer);
}

void MainWindow_GUI::create_PI_Buttons()
{
  PI_ButtonGroup = new QGroupBox("Панель управления");
  PI_ButtonGroup->setAlignment(Qt::AlignCenter);
  PI_MainLayout->addWidget(PI_ButtonGroup);

  PI_ButtonLayout = new QVBoxLayout();
  PI_ButtonGroup->setLayout(PI_ButtonLayout);

  PI_PushButtonPersonalize = new QPushButton("Персонализировать");
  PI_ButtonLayout->addWidget(PI_PushButtonPersonalize);

  PI_ButtonSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  PI_ButtonLayout->addItem(PI_ButtonSpacer);

  PI_PushButtonHardReset = new QPushButton("Сброс");
  PI_ButtonLayout->addWidget(PI_PushButtonHardReset);
}

void MainWindow_GUI::create_PI_ObuDataView()
{
  PI_ObuData = new QGroupBox("Данные транспондера");
  PI_ObuData->setAlignment(Qt::AlignCenter);
  PI_MainLayout->addWidget(PI_ObuData);

  PI_ObuDataLayout = new QHBoxLayout();
  PI_ObuData->setLayout(PI_ObuDataLayout);

  PI_MI_SystemElementView = new QTableView();
  PI_ObuDataLayout->addWidget(PI_MI_SystemElementView);

  PI_EfcElementView = new QTableView();
  PI_ObuDataLayout->addWidget(PI_EfcElementView);
}

// Настройка вкладок функциональности
void MainWindow_GUI::create_MI_Tabs(void)
{
  MI_Tabs = new QTabWidget();
  MI_Tabs->setObjectName(QString::fromUtf8("MI_Tabs"));
  MI_MainLayout->addWidget(MI_Tabs);

  // Настраиваем стартовую страницу
  MI_Tabs->setCurrentIndex(0);

  // Контруируем вкладку для настройки DTR
  createDtrTab();

  // Конструируем вкладку для персонализации
  createPersonalizationTab();

  // Конструируем вкладку тестирования
  createTestingTab();

  // Конструируем вкладку энергопотребления
  createPowerConsumptionTab();

  // Конструируем вкладку безопасности
  createSecurityTab();

  // Конструируем вкладку настроек
  createSettingsTab();
}

void MainWindow_GUI::createDtrTab()
{
  DtrTab = new QWidget();
  DtrTab->setObjectName(QString::fromUtf8("DtrTab"));
  MI_Tabs->addTab(DtrTab, "Ридер");

  // Основной макет для интерфейса DTR
  DtrMainLayout = new QHBoxLayout();
  DtrTab->setLayout(DtrMainLayout);

  // Панель управления
  DtrControlPanelGroupBox = new QGroupBox(QString("Панель управления"));
  DtrControlPanelGroupBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  DtrMainLayout->addWidget(DtrControlPanelGroupBox);

  // Макет для виджетов панели управления
  DtrControlPanelLayout = new QVBoxLayout();
  DtrControlPanelGroupBox->setLayout(DtrControlPanelLayout);

  // Кнопки
  PushButtonConnect = new QPushButton("Подключиться");
  DtrControlPanelLayout->addWidget(PushButtonConnect);

  PushButtonDisconnect = new QPushButton("Отключиться");
  PushButtonDisconnect->setObjectName(
      QString::fromUtf8("PushButtonDisconnect"));
  DtrControlPanelLayout->addWidget(PushButtonDisconnect);

  PushButtonReboot = new QPushButton("Перезагрузить");
  DtrControlPanelLayout->addWidget(PushButtonReboot);

  PushButtonStatus = new QPushButton("Статус");
  DtrControlPanelLayout->addWidget(PushButtonStatus);

  PushButtonTransmitCustomData =
      new QPushButton("Отправить произвольные данные");
  PushButtonTransmitCustomData->setObjectName(
      QString::fromUtf8("PushButtonTransmitCustomData"));
  DtrControlPanelLayout->addWidget(PushButtonTransmitCustomData);

  LineEditCustomData = new QLineEdit("Введите произвольные данные");
  DtrControlPanelLayout->addWidget(LineEditCustomData);

  DtrButtonLayoutVS =
      new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DtrControlPanelLayout->addItem(DtrButtonLayoutVS);

  // Отображаем виджеты специфичные для каждого из возможных DTR
  if (CurrentDtr == MD5859)
    createMD5859SpecificWidgets();
  else
    createTC278SpecificWidgets();

  // Представления для отображения параметров DTR
  DtrParametersView = new QTableView();
  DtrParametersView->setObjectName(QString::fromUtf8("DtrParametersView"));
  DtrMainLayout->addWidget(DtrParametersView);

  // Настройка пропорции между объектами на макете
  DtrMainLayout->setStretch(0, 1);
  DtrMainLayout->setStretch(1, 3);
}

void MainWindow_GUI::createTC278SpecificWidgets()
{
  PushButtonEnableSynchronization = new QPushButton("Включить синхронизацию");
  DtrControlPanelLayout->addWidget(PushButtonEnableSynchronization);

  PushButtonDisableSynchronization = new QPushButton("Выключить синхронизацию");
  DtrControlPanelLayout->addWidget(PushButtonDisableSynchronization);

  PushButtonSetAttenuation = new QPushButton("Установить мощность сигнала");
  DtrControlPanelLayout->addWidget(PushButtonSetAttenuation);

  PushButtonGetAttenuation = new QPushButton("Считать текущую мощность радиосигнала");
  DtrControlPanelLayout->addWidget(PushButtonGetAttenuation);
}

void MainWindow_GUI::createMD5859SpecificWidgets()
{
  PushButtonEnableCommunication = new QPushButton("Включить коммуникацию с транспондером");
  DtrControlPanelLayout->addWidget(PushButtonEnableCommunication);

  PushButtonDisableCommunication = new QPushButton("Выключить коммуникацию с транспондером");
  DtrControlPanelLayout->addWidget(PushButtonDisableCommunication);

  PushButtonConfigure = new QPushButton("Обновление конфигурации");
  DtrControlPanelLayout->addWidget(PushButtonConfigure);
}

// Конструирование графического интерфейса персонализации
void MainWindow_GUI::createPersonalizationTab(void)
{
  PersonalizationTab = new QWidget();
  MI_Tabs->addTab(PersonalizationTab, "Персонализация");

  // Оcновной макет интерфейса персонализации
  PersonalizationMainLayout = new QHBoxLayout();
  PersonalizationTab->setLayout(PersonalizationMainLayout);

  // Вспомогательный макет для основного вида
  PersonalizationMainSubLayout = new QVBoxLayout();
  PersonalizationMainLayout->addLayout(PersonalizationMainSubLayout);

  // Виджет для представления данных транспондера
  MI_ObuDataGroupBox = new QGroupBox(QString("Данные транспондера"));
  MI_ObuDataGroupBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  PersonalizationMainSubLayout->addWidget(MI_ObuDataGroupBox);

  // Макет для представлений
  MI_ObuDataViewLayout = new QHBoxLayout();
  MI_ObuDataGroupBox->setLayout(MI_ObuDataViewLayout);

  // Представления для отображения данных транспондера
  MI_SystemElementView = new QTableView();
  MI_ObuDataViewLayout->addWidget(MI_SystemElementView);

  MI_EfcElementView = new QTableView();
  MI_ObuDataViewLayout->addWidget(MI_EfcElementView);

  // Главный макет для виджетов пользовательского ввода
  PersonalizationControlPanelGroupBox =
      new QGroupBox(QString("Панель управления"));
  PersonalizationControlPanelGroupBox->setAlignment(Qt::AlignHCenter |
                                                    Qt::AlignVCenter);
  PersonalizationMainSubLayout->addWidget(PersonalizationControlPanelGroupBox);

  PersonalizationButtonMainLayout = new QHBoxLayout();
  PersonalizationControlPanelGroupBox->setLayout(
      PersonalizationButtonMainLayout);

  // Настройка пропорции между объектами на макете
  PersonalizationMainSubLayout->setStretch(0, 1);
  PersonalizationMainSubLayout->setStretch(1, 0);

  // Макет для виджетов персонализации
  PersonalizationButtonSubLayout1 = new QVBoxLayout();
  PersonalizationButtonMainLayout->addLayout(PersonalizationButtonSubLayout1);

  // Виджеты для персонализации
  PushButtonRead = new QPushButton("Считывание");
  PersonalizationButtonSubLayout1->addWidget(PushButtonRead);

  MI_PushButtonPersonalize = new QPushButton("Персонализация");
  PersonalizationButtonSubLayout1->addWidget(MI_PushButtonPersonalize);

  PersonalizationVerticalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  PersonalizationButtonSubLayout1->addItem(PersonalizationVerticalSpacer1);

  // Макет для виджетов очистки данных транспондера
  PersonalizationButtonSubLayout2 = new QVBoxLayout();
  PersonalizationButtonMainLayout->addLayout(PersonalizationButtonSubLayout2);

  // Виджеты для очистки данных транспондера
  PushButtonClear = new QPushButton("Очистка");
  PersonalizationButtonSubLayout2->addWidget(PushButtonClear);

  MI_PushButtonHardReset = new QPushButton("Аппаратный сброс");
  PersonalizationButtonSubLayout2->addWidget(MI_PushButtonHardReset);

  PersonalizationVerticalSpacer2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  PersonalizationButtonSubLayout2->addItem(PersonalizationVerticalSpacer2);

  // Макет для виджетов ручной перезаписи атрибутов
  PersonalizationButtonSubLayout3 = new QVBoxLayout();
  PersonalizationButtonMainLayout->addLayout(PersonalizationButtonSubLayout3);

  // Настройка пропорции между объектами на макете
  PersonalizationButtonMainLayout->setStretch(0, 1);
  PersonalizationButtonMainLayout->setStretch(1, 1);
  PersonalizationButtonMainLayout->setStretch(2, 1);

  // Кнопки для ручной перезаписи атрибутов
  ElementChoice = new QComboBox();
  PersonalizationButtonSubLayout3->addWidget(ElementChoice);

  AttributeChoice = new QComboBox();
  PersonalizationButtonSubLayout3->addWidget(AttributeChoice);

  AttributeNewValue = new QLineEdit(QString("Новое значение атрибута"));
  PersonalizationButtonSubLayout3->addWidget(AttributeNewValue);

  PushButtonOverwriteAttribute = new QPushButton("Перезаписать атрибут");
  PersonalizationButtonSubLayout3->addWidget(PushButtonOverwriteAttribute);

  PersonalizationVerticalSpacer3 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  PersonalizationButtonSubLayout3->addItem(PersonalizationVerticalSpacer3);
}

void MainWindow_GUI::createTestingTab(void)
{
  TestingTab = new QWidget();
  PersonalizationTab->setObjectName(QString::fromUtf8("TestingTab"));
  MI_Tabs->addTab(TestingTab, "Тестирование");

  // Основной макет интерфейса тестирования
  TestingMainLayout = new QHBoxLayout();
  TestingTab->setLayout(TestingMainLayout);

  // Панель управления
  TestingControlPanelGroupBox = new QGroupBox(QString("Панель управления"));
  TestingControlPanelGroupBox->setAlignment(Qt::AlignHCenter |
                                            Qt::AlignVCenter);
  TestingMainLayout->addWidget(TestingControlPanelGroupBox);

  TestingControlPanelLayout = new QVBoxLayout();
  TestingControlPanelGroupBox->setLayout(TestingControlPanelLayout);

  PushButtonFullTesting = new QPushButton("Полное тестирование");
  TestingControlPanelLayout->addWidget(PushButtonFullTesting);

  PushButtonAppIKTest = new QPushButton("Тест ядра инициализации");
  TestingControlPanelLayout->addWidget(PushButtonAppIKTest);

  PushButtonAppTKTest = new QPushButton("Тест ядра передачи");
  TestingControlPanelLayout->addWidget(PushButtonAppTKTest);

  PushButtonEfcTest = new QPushButton("Тест приложения EFC");
  TestingControlPanelLayout->addWidget(PushButtonEfcTest);

  PushButtonSystemTest = new QPushButton("Тест системного приложения");
  TestingControlPanelLayout->addWidget(PushButtonSystemTest);

  PushButtonSingleTest = new QPushButton("Одиночное тестирование");
  TestingControlPanelLayout->addWidget(PushButtonSingleTest);

  LineEditTestName = new QLineEdit("Введите код теста");
  TestingControlPanelLayout->addWidget(LineEditTestName);

  VerticalSpacer1 =
      new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  TestingControlPanelLayout->addItem(VerticalSpacer1);

  PushButtonDsrcTransaction = new QPushButton("CARDME4 Транзакция");
  TestingControlPanelLayout->addWidget(PushButtonDsrcTransaction);

  // Представление результатов тестирования
  TestResultsGroupBox = new QGroupBox(QString("Результаты тестирования"));
  TestResultsGroupBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  TestingMainLayout->addWidget(TestResultsGroupBox);

  TestGridLayout = new QGridLayout();
  TestResultsGroupBox->setLayout(TestGridLayout);

  AppIKTestsLabel = new QLabel("Ядро инициализации");
  AppIKTestsLabel->setAlignment(Qt::AlignCenter);
  TestGridLayout->addWidget(AppIKTestsLabel, 0, 0, 1, 1);

  AppTKTestsLabel = new QLabel("Ядро передачи");
  AppTKTestsLabel->setAlignment(Qt::AlignCenter);
  TestGridLayout->addWidget(AppTKTestsLabel, 0, 1, 1, 1);

  EfcTestsLabel = new QLabel("Приложение EFC");
  EfcTestsLabel->setAlignment(Qt::AlignCenter);
  TestGridLayout->addWidget(EfcTestsLabel, 0, 2, 1, 1);

  SystemTestsLabel = new QLabel("Системное приложение");
  SystemTestsLabel->setAlignment(Qt::AlignCenter);
  TestGridLayout->addWidget(SystemTestsLabel, 0, 3, 1, 1);

  AppIKTestsView = new QTableView();
  TestTableView_ColorDelegate *AppIKTestsColorDelegate =
      new TestTableView_ColorDelegate(AppIKTestsView);
  AppIKTestsView->setItemDelegate(AppIKTestsColorDelegate);
  TestGridLayout->addWidget(AppIKTestsView, 1, 0, 1, 1);

  AppTKTestsView = new QTableView();
  TestTableView_ColorDelegate *AppTKTestsColorDelegate =
      new TestTableView_ColorDelegate(AppTKTestsView);
  AppTKTestsView->setItemDelegate(AppTKTestsColorDelegate);
  TestGridLayout->addWidget(AppTKTestsView, 1, 1, 1, 1);

  EfcTestsView = new QTableView();
  TestTableView_ColorDelegate *EfcTestsColorDelegate =
      new TestTableView_ColorDelegate(EfcTestsView);
  EfcTestsView->setItemDelegate(EfcTestsColorDelegate);
  TestGridLayout->addWidget(EfcTestsView, 1, 2, 1, 1);

  SystemTestsView = new QTableView();
  TestTableView_ColorDelegate *SystemTestsColorDelegate =
      new TestTableView_ColorDelegate(SystemTestsView);
  SystemTestsView->setItemDelegate(SystemTestsColorDelegate);
  TestGridLayout->addWidget(SystemTestsView, 1, 3, 1, 1);

  // Настройка пропорции между объектами на макете
  TestingMainLayout->setStretch(0, 1);
  TestingMainLayout->setStretch(1, 0);
}

void MainWindow_GUI::createPowerConsumptionTab(void)
{
  SupplyVoltageTab = new QWidget();
  SupplyVoltageTab->setObjectName(QString::fromUtf8("SupplyVoltageTab"));
  MI_Tabs->addTab(SupplyVoltageTab, "Аккумулятор");

  SVMainLayout = new QHBoxLayout();
  SupplyVoltageTab->setLayout(SVMainLayout);

  // Панель управления
  SVControlPanelGroupBox = new QGroupBox(QString("Панель управления"));
  SVControlPanelGroupBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  SVMainLayout->addWidget(SVControlPanelGroupBox);

  SVControlPanelLayout = new QVBoxLayout();
  SVControlPanelGroupBox->setLayout(SVControlPanelLayout);

  PushButtonStartEndlessMeasuring = new QPushButton("Начать измерение");
  SVControlPanelLayout->addWidget(PushButtonStartEndlessMeasuring);

  PushButtonStopEndlessMeasuring = new QPushButton("Остановить измерение");
  SVControlPanelLayout->addWidget(PushButtonStopEndlessMeasuring);

  VerticalSpacer2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  SVControlPanelLayout->addItem(VerticalSpacer2);

  LineEditMeasureCount = new QLineEdit("Количество замеров");
  SVControlPanelLayout->addWidget(LineEditMeasureCount);

  PushButtonMeasureCountableTimes = new QPushButton("Провести замеры");
  SVControlPanelLayout->addWidget(PushButtonMeasureCountableTimes);

  // График иземерений
  BatteryVoltageChartView = new QChartView();
  BatteryVoltageChartView->setRenderHint(QPainter::Antialiasing);
  SVMainLayout->addWidget(BatteryVoltageChartView);

  // Настройка пропорции между объектами на макете
  SVMainLayout->setStretch(0, 0);
  SVMainLayout->setStretch(1, 1);
}

void MainWindow_GUI::createSecurityTab(void)
{
  SecurityTab = new QWidget();
  SecurityTab->setObjectName(QString::fromUtf8("SecurityTab"));
  MI_Tabs->addTab(SecurityTab, "Безопасность");

  // Представления групп ключей
  SecurityLayout = new QGridLayout();
  SecurityTab->setLayout(SecurityLayout);

  TMasterKeysLabel = new QLabel("Транспортные мастер ключи");
  TMasterKeysLabel->setAlignment(Qt::AlignCenter);
  SecurityLayout->addWidget(TMasterKeysLabel, 0, 0, 1, 1);

  CMasterKeysLabel = new QLabel("Коммерческие мастер ключи");
  CMasterKeysLabel->setAlignment(Qt::AlignCenter);
  SecurityLayout->addWidget(CMasterKeysLabel, 0, 1, 1, 1);

  CommonKeysLabel = new QLabel("Ключи транспондера");
  CommonKeysLabel->setAlignment(Qt::AlignCenter);
  SecurityLayout->addWidget(CommonKeysLabel, 0, 2, 1, 1);

  TMasterKeysView = new QTableView();
  TMasterKeysView->setObjectName(QString::fromUtf8("TMasterKeysView"));
  SecurityLayout->addWidget(TMasterKeysView, 1, 0, 1, 1);

  CMasterKeysView = new QTableView();
  CMasterKeysView->setObjectName(QString::fromUtf8("CMasterKeysView"));
  SecurityLayout->addWidget(CMasterKeysView, 1, 1, 1, 1);

  CommonKeysView = new QTableView();
  CommonKeysView->setObjectName(QString::fromUtf8("CommonKeysView"));
  SecurityLayout->addWidget(CommonKeysView, 1, 2, 1, 1);
}

void MainWindow_GUI::createSettingsTab()
{
  SettingsTab = new QWidget();
  SettingsTab->setObjectName(QString::fromUtf8("SecurityTab"));
  MI_Tabs->addTab(SettingsTab, "Настройки");

  // Главный макет меню настроек
  SettingsMainLayout = new QHBoxLayout();
  SettingsTab->setLayout(SettingsMainLayout);

  SettingsMainSubLayout = new QVBoxLayout();
  SettingsMainLayout->addLayout(SettingsMainSubLayout);

  SettingsHorizontalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  SettingsMainLayout->addItem(SettingsHorizontalSpacer1);

  // Настройки персонализации
  QGroupBox *PersoSettingsGroupBox =
      new QGroupBox(QString("Настройки персонализации"));
  SettingsMainSubLayout->addWidget(PersoSettingsGroupBox);

  PersoSettingsMainLayout = new QGridLayout();
  PersoSettingsGroupBox->setLayout(PersoSettingsMainLayout);

  UseServerPersoLabel = new QLabel("Серверная персонализация");
  PersoSettingsMainLayout->addWidget(UseServerPersoLabel, 0, 0, 1, 1);

  UseServerPersoCheckBox = new QCheckBox("Вкл/выкл");
  PersoSettingsMainLayout->addWidget(UseServerPersoCheckBox, 0, 1, 1, 1);

  ServerCommonKeyGenerationLabel =
      new QLabel("Серверная генерация ключей транспондера");
  PersoSettingsMainLayout->addWidget(ServerCommonKeyGenerationLabel, 1, 0, 1,
                                     1);

  ServerCommonKeyGenerationCheckBox = new QCheckBox("Вкл/выкл");
  PersoSettingsMainLayout->addWidget(ServerCommonKeyGenerationCheckBox, 1, 1, 1,
                                     1);

  ipAddressPersoServerLabel =
      new QLabel("IP адрес или URL сервера персонализации");
  PersoSettingsMainLayout->addWidget(ipAddressPersoServerLabel, 2, 0, 1, 1);

  ipAddressPersoServerLineEdit = new QLineEdit("10.0.1.1");
  PersoSettingsMainLayout->addWidget(ipAddressPersoServerLineEdit, 2, 1, 1, 1);

  localMasterKeyPathLabel = new QLabel("Расположение локальных мастер ключей");
  PersoSettingsMainLayout->addWidget(localMasterKeyPathLabel, 3, 0, 1, 1);

  localMasterKeyPathLineEdit = new QLineEdit("");
  PersoSettingsMainLayout->addWidget(localMasterKeyPathLineEdit, 3, 1, 1, 1);

  // Кнопка сохранения настроек
  PushButtonSaveSettings = new QPushButton("Применить изменения");
  SettingsMainSubLayout->addWidget(PushButtonSaveSettings);

  // Сжимаем позиционирование
  SettingsVerticalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  SettingsMainSubLayout->addItem(SettingsVerticalSpacer1);
}

void MainWindow_GUI::create_MI_LogWidgets()
{
  GroupLog = new QGroupBox("Глобальный лог");
  GroupLog->setAlignment(Qt::AlignCenter);
  MI_MainLayout->addWidget(GroupLog);

  LogLayout = new QHBoxLayout();
  GroupLog->setLayout(LogLayout);

  GeneralLogs = new QPlainTextEdit();
  GeneralLogs->setEnabled(true);
  GeneralLogs->setTabletTracking(true);
  GeneralLogs->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  GeneralLogs->setCenterOnScroll(false);
  LogLayout->addWidget(GeneralLogs);
}
