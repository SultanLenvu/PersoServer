#include "gui_master.h"

GUI_Master::GUI_Master(QObject* parent) : GUI(parent, Master) {
  setObjectName("GUI_Master");
}

QWidget* GUI_Master::create() {
  // Создаем вкладки мастер меню
  createTabs();

  // Создаем виджеты логов
  createLog();

  // Настраиваем пропорции отображаемых элементов
  MainLayout->setStretch(0, 2);
  MainLayout->setStretch(1, 1);

  return MainWidget;
}

void GUI_Master::update() {
  DataBaseBufferView->resizeColumnsToContents();
  DataBaseBufferView->update();

  NewObuListTableView->resizeColumnsToContents();
  NewObuListTableView->update();
}

void GUI_Master::createTabs() {
  Tabs = new QTabWidget();
  MainLayout->addWidget(Tabs);

  // Задаем стартовую страницу
  Tabs->setCurrentIndex(0);

  // Контруируем вкладку для настройки DTR
  createDataBaseTab();

  // Конструируем вкладку интерфейса инициализации транспондеров
  createObuInitializationTab();

  // Конструируем вкладку настроек
  createSettingsTab();
}

void GUI_Master::createDataBaseTab() {
  DataBaseTab = new QWidget();
  Tabs->addTab(DataBaseTab, "База данных");

  // Основной макет для интерфейса DTR
  DataBaseMainLayout = new QHBoxLayout();
  DataBaseTab->setLayout(DataBaseMainLayout);

  // Панель управления
  DataBaseControlPanelGroup = new QGroupBox(QString("Панель управления"));
  DataBaseControlPanelGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  DataBaseMainLayout->addWidget(DataBaseControlPanelGroup);

  DataBaseControlPanelLayout = new QVBoxLayout();
  DataBaseControlPanelGroup->setLayout(DataBaseControlPanelLayout);

  // Кнопки
  ConnectDataBasePushButton = new QPushButton("Подключиться");
  DataBaseControlPanelLayout->addWidget(ConnectDataBasePushButton);

  DisconnectDataBasePushButton = new QPushButton("Отключиться");
  DataBaseControlPanelLayout->addWidget(DisconnectDataBasePushButton);

  PushButtonLayoutVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DataBaseControlPanelLayout->addItem(PushButtonLayoutVS);

  CustomRequestLineEdit = new QLineEdit();
  DataBaseControlPanelLayout->addWidget(CustomRequestLineEdit);

  TransmitCustomRequestPushButton = new QPushButton("Отправить команду");
  DataBaseControlPanelLayout->addWidget(TransmitCustomRequestPushButton);

  // Отображение буфера считанных данных из БД
  DataBaseBufferGroup = new QGroupBox(QString("Буффер считанных данных"));
  DataBaseBufferGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  DataBaseMainLayout->addWidget(DataBaseBufferGroup);

  DataBaseControlPanelLayout = new QVBoxLayout();
  DataBaseBufferGroup->setLayout(DataBaseControlPanelLayout);

  DataBaseBufferView = new QTableView();
  DataBaseControlPanelLayout->addWidget(DataBaseBufferView);

  // Настройка пропорции между объектами на макете
  DataBaseMainLayout->setStretch(0, 1);
  DataBaseMainLayout->setStretch(1, 3);
}

void GUI_Master::createObuInitializationTab() {
  ObuInitTab = new QWidget();
  Tabs->addTab(ObuInitTab, "Инициализация");

  // Основной макет для интерфейса инициализации транспондеров
  ObuInitTabMainLayout = new QHBoxLayout();
  ObuInitTab->setLayout(ObuInitTabMainLayout);

  // Панель управления инициализацией транспондеров
  ObuInitControlPanel = new QGroupBox("Панель управления");
  ObuInitTabMainLayout->addWidget(ObuInitControlPanel);

  ObuInitControlPanelLayout = new QVBoxLayout();
  ObuInitControlPanel->setLayout(ObuInitControlPanelLayout);

  PanFormatRadioButton = new QRadioButton("Формат PAN");
  ObuInitControlPanelLayout->addWidget(PanFormatRadioButton);
  PanFormatRadioButton = new QRadioButton("Формат SN+PAN");
  ObuInitControlPanelLayout->addWidget(PanFormatRadioButton);

  ObuInitControlPanelSubLayout = new QHBoxLayout();
  ObuInitControlPanelLayout->addLayout(ObuInitControlPanelSubLayout);

  InitFilePathLabel = new QLabel("Путь к файлу инициализации");
  ObuInitControlPanelSubLayout->addWidget(InitFilePathLabel);

  InitFilePathLineEdit = new QLineEdit();
  ObuInitControlPanelSubLayout->addWidget(InitFilePathLineEdit);
  InitNewObuListPushButton = new QPushButton("Обзор");
  ObuInitControlPanelSubLayout->addWidget(InitNewObuListPushButton);

  InitNewObuListPushButton = new QPushButton("Инициализировать транспондеры");
  ObuInitControlPanelLayout->addWidget(InitNewObuListPushButton);

  ObuInitControlPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  ObuInitControlPanelLayout->addItem(ObuInitControlPanelVS);

  // Панель управления инициализацией транспондеров
  NewObuListPanel = new QGroupBox("Список инициализации");
  ObuInitTabMainLayout->addWidget(NewObuListPanel);

  NewObuListLayout = new QVBoxLayout();
  NewObuListPanel->setLayout(NewObuListLayout);

  NewObuListTableView = new QTableView();
  NewObuListLayout->addWidget(NewObuListTableView);

  // Сжатие по горизонтали
  ObuInitTabMainLayoutHS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  ObuInitTabMainLayout->addItem(ObuInitTabMainLayoutHS);

  // Настройка пропорции между объектами на макете
  ObuInitTabMainLayout->setStretch(0, 2);
  ObuInitTabMainLayout->setStretch(1, 2);
}

void GUI_Master::createSecurityTab(void) {
  SecurityTab = new QWidget();
  Tabs->addTab(SecurityTab, "Безопасность");

  // Представления групп ключей
  SecurityTabMainLayout = new QGridLayout();
  SecurityTab->setLayout(SecurityTabMainLayout);

  TMasterKeysLabel = new QLabel("Транспортные мастер ключи");
  TMasterKeysLabel->setAlignment(Qt::AlignCenter);
  SecurityTabMainLayout->addWidget(TMasterKeysLabel, 0, 0, 1, 1);

  CMasterKeysLabel = new QLabel("Коммерческие мастер ключи");
  CMasterKeysLabel->setAlignment(Qt::AlignCenter);
  SecurityTabMainLayout->addWidget(CMasterKeysLabel, 0, 1, 1, 1);

  CommonKeysLabel = new QLabel("Ключи транспондера");
  CommonKeysLabel->setAlignment(Qt::AlignCenter);
  SecurityTabMainLayout->addWidget(CommonKeysLabel, 0, 2, 1, 1);

  TMasterKeysView = new QTableView();
  TMasterKeysView->setObjectName(QString::fromUtf8("TMasterKeysView"));
  SecurityTabMainLayout->addWidget(TMasterKeysView, 1, 0, 1, 1);

  CMasterKeysView = new QTableView();
  CMasterKeysView->setObjectName(QString::fromUtf8("CMasterKeysView"));
  SecurityTabMainLayout->addWidget(CMasterKeysView, 1, 1, 1, 1);

  CommonKeysView = new QTableView();
  CommonKeysView->setObjectName(QString::fromUtf8("CommonKeysView"));
  SecurityTabMainLayout->addWidget(CommonKeysView, 1, 2, 1, 1);
}

void GUI_Master::createSettingsTab() {
  SettingsTab = new QWidget();
  Tabs->addTab(SettingsTab, "Настройки");

  // Главный макет меню настроек
  SettingsMainLayout = new QHBoxLayout();
  SettingsTab->setLayout(SettingsMainLayout);

  SettingsMainSubLayout = new QVBoxLayout();
  SettingsMainLayout->addLayout(SettingsMainSubLayout);

  SettingsHorizontalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  SettingsMainLayout->addItem(SettingsHorizontalSpacer1);

  // Настройки персонализации
  QGroupBox* PersoSettingsGroupBox =
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

  PersoServerIpAddressLabel =
      new QLabel("IP адрес или URL сервера персонализации");
  PersoSettingsMainLayout->addWidget(PersoServerIpAddressLabel, 2, 0, 1, 1);

  PersoServerIpAddressLineEdit = new QLineEdit();
  PersoSettingsMainLayout->addWidget(PersoServerIpAddressLineEdit, 2, 1, 1, 1);

  PersoServerPortLabel = new QLabel("Порт сервера персонализации");
  PersoSettingsMainLayout->addWidget(PersoServerPortLabel, 3, 0, 1, 1);

  PersoServerPortLineEdit = new QLineEdit();
  PersoSettingsMainLayout->addWidget(PersoServerPortLineEdit, 3, 1, 1, 1);

  localMasterKeyPathLabel = new QLabel("Расположение локальных мастер ключей");
  PersoSettingsMainLayout->addWidget(localMasterKeyPathLabel, 4, 0, 1, 1);

  localMasterKeyPathLineEdit = new QLineEdit("");
  PersoSettingsMainLayout->addWidget(localMasterKeyPathLineEdit, 4, 1, 1, 1);

  // Кнопка сохранения настроек
  ApplySettingsPushButton = new QPushButton("Применить изменения");
  SettingsMainSubLayout->addWidget(ApplySettingsPushButton);

  // Сжимаем позиционирование
  SettingsVerticalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  SettingsMainSubLayout->addItem(SettingsVerticalSpacer1);
}
