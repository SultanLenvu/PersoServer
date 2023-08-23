#include "gui_master.h"

MasterGUI::MasterGUI(QObject* parent) : GUI(parent, Master) {
  setObjectName("MasterGUI");
}

QWidget* MasterGUI::create() {
  // Создаем вкладки мастер меню
  createTabs();

  // Создаем виджеты логов
  createLog();

  // Настраиваем пропорции отображаемых элементов
  MainLayout->setStretch(0, 2);
  MainLayout->setStretch(1, 1);

  return MainWidget;
}

void MasterGUI::update() {
  DataBaseBufferView->resizeColumnsToContents();
  DataBaseBufferView->update();

  PanListTableView->resizeColumnsToContents();
  PanListTableView->update();
}

void MasterGUI::createTabs() {
  Tabs = new QTabWidget();
  MainLayout->addWidget(Tabs);

  // Задаем стартовую страницу
  Tabs->setCurrentIndex(0);

  // Конструируем вкладку для управления сервером
  createServerTab();

  // Контруируем вкладку для взаимодействия с базой данных
  createDataBaseTab();

  // Конструируем вкладку интерфейса для создания заказов
  createOrderCreationTab();

  // Конструируем вкладку интерфейса для управления безопасностью
  createSecurityTab();

  // Конструируем вкладку настроек
  createSettingsTab();
}

void MasterGUI::createServerTab() {}

void MasterGUI::createDataBaseTab() {
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

  PushButtonLayoutVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DataBaseControlPanelLayout->addItem(PushButtonLayoutVS1);

  ShowProductionLineTablePushButton = new QPushButton("Линии производства");
  DataBaseControlPanelLayout->addWidget(ShowProductionLineTablePushButton);

  ShowTransponderTablePushButton = new QPushButton("Транспондеры");
  DataBaseControlPanelLayout->addWidget(ShowTransponderTablePushButton);

  ShowOrderTablePushButton = new QPushButton("Заказы");
  DataBaseControlPanelLayout->addWidget(ShowOrderTablePushButton);

  ShowIssuerTablePushButton = new QPushButton("Заказчики");
  DataBaseControlPanelLayout->addWidget(ShowIssuerTablePushButton);

  ShowBoxTablePushButton = new QPushButton("Боксы");
  DataBaseControlPanelLayout->addWidget(ShowBoxTablePushButton);

  ShowPalletPushButton = new QPushButton("Палеты");
  DataBaseControlPanelLayout->addWidget(ShowPalletPushButton);

  PushButtonLayoutVS2 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DataBaseControlPanelLayout->addItem(PushButtonLayoutVS2);

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

void MasterGUI::createOrderCreationTab() {
  OrderCreationTab = new QWidget();
  Tabs->addTab(OrderCreationTab, "Создание заказов");

  // Основной макет интерфейса для создания заказов
  OrderCreationTabMainLayout = new QHBoxLayout();
  OrderCreationTab->setLayout(OrderCreationTabMainLayout);

  // Панель управления для создания заказов
  OrderCreationControlPanel = new QGroupBox("Панель управления");
  OrderCreationTabMainLayout->addWidget(OrderCreationControlPanel);

  OrderCreationControlPanelLayout = new QVBoxLayout();
  OrderCreationControlPanel->setLayout(OrderCreationControlPanelLayout);

  FullPersonalizationCheckBox = new QCheckBox("Полная персонализация");
  OrderCreationControlPanelLayout->addWidget(FullPersonalizationCheckBox);
  connect(FullPersonalizationCheckBox, &QCheckBox::stateChanged, this,
          &MasterGUI::on_FullPersonalizationCheckBoxChanged);

  OrderCreationControlPanelSubLayout1 = new QHBoxLayout();
  OrderCreationControlPanelLayout->addLayout(
      OrderCreationControlPanelSubLayout1);

  IssuerNameComboLabel = new QLabel("Компания заказчик");
  OrderCreationControlPanelSubLayout1->addWidget(IssuerNameComboLabel);

  IssuerNameComboBox = new QComboBox();
  IssuerNameComboBox->addItem("Новое качество дорог");
  IssuerNameComboBox->addItem("Западный скоростной диаметр");
  OrderCreationControlPanelSubLayout1->addWidget(IssuerNameComboBox);

  OrderCreationControlPanelSubLayout2 = new QHBoxLayout();
  OrderCreationControlPanelLayout->addLayout(
      OrderCreationControlPanelSubLayout2);

  TransponderQuantityLabel = new QLabel("Количество транспондеров");
  OrderCreationControlPanelSubLayout2->addWidget(TransponderQuantityLabel);

  TransponderQuantityLineEdit = new QLineEdit("100");
  OrderCreationControlPanelSubLayout2->addWidget(TransponderQuantityLineEdit);

  CreateNewOrderPushButton = new QPushButton("Создать новый заказ");
  OrderCreationControlPanelLayout->addWidget(CreateNewOrderPushButton);

  OrderCreationControlPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  OrderCreationControlPanelLayout->addItem(OrderCreationControlPanelVS);

  // Панель управления инициализацией транспондеров
  PanListPanel = new QGroupBox("Список новых PAN");
  OrderCreationTabMainLayout->addWidget(PanListPanel);

  PanListLayout = new QVBoxLayout();
  PanListPanel->setLayout(PanListLayout);

  PanListTableView = new QTableView();
  PanListLayout->addWidget(PanListTableView);

  // Сжатие по горизонтали
  OrderCreationTabMainLayoutHS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  OrderCreationTabMainLayout->addItem(OrderCreationTabMainLayoutHS);

  // Настройка пропорции между объектами на макете
  OrderCreationTabMainLayout->setStretch(0, 2);
  OrderCreationTabMainLayout->setStretch(1, 2);
}

void MasterGUI::createSecurityTab(void) {
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

void MasterGUI::createSettingsTab() {
  QSettings settings;
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

  // Настройки сервера
  PersoServerSettingsGroupBox = new QGroupBox(QString("Настройки сервера"));
  SettingsMainSubLayout->addWidget(PersoServerSettingsGroupBox);

  PersoServerSettingsLayout = new QGridLayout();
  PersoServerSettingsGroupBox->setLayout(PersoServerSettingsLayout);

  PersoServerIpLabel = new QLabel("IP-адрес ");
  PersoServerSettingsLayout->addWidget(PersoServerIpLabel, 0, 0, 1, 1);

  PersoServerIpLineEdit =
      new QLineEdit(settings.value("PersoHost/Ip").toString());
  PersoServerSettingsLayout->addWidget(PersoServerIpLineEdit, 0, 1, 1, 1);

  PersoServerPortLabel = new QLabel("Порт ");
  PersoServerSettingsLayout->addWidget(PersoServerPortLabel, 1, 0, 1, 1);

  PersoServerPortLineEdit =
      new QLineEdit(settings.value("PersoHost/Port").toString());
  PersoServerSettingsLayout->addWidget(PersoServerPortLineEdit, 1, 1, 1, 1);

  // Настройки базы данных
  DatabaseSettingsGroupBox = new QGroupBox(QString("Настройки базы данных"));
  SettingsMainSubLayout->addWidget(DatabaseSettingsGroupBox);

  DatabaseSettingsLayout = new QGridLayout();
  DatabaseSettingsGroupBox->setLayout(DatabaseSettingsLayout);

  DatabaseIpLabel = new QLabel("IP-адрес");
  DatabaseSettingsLayout->addWidget(DatabaseIpLabel, 0, 0, 1, 1);

  DatabaseIpLineEdit =
      new QLineEdit(settings.value("Database/Server/Ip").toString());
  DatabaseSettingsLayout->addWidget(DatabaseIpLineEdit, 0, 1, 1, 1);

  DatabasePortLabel = new QLabel("Порт ");
  DatabaseSettingsLayout->addWidget(DatabasePortLabel, 1, 0, 1, 1);

  DatabasePortLineEdit =
      new QLineEdit(settings.value("Database/Server/Port").toString());
  DatabaseSettingsLayout->addWidget(DatabasePortLineEdit, 1, 1, 1, 1);

  DatabaseNameLabel = new QLabel("Название базы данных ");
  DatabaseSettingsLayout->addWidget(DatabaseNameLabel, 2, 0, 1, 1);

  DatabaseNameLineEdit =
      new QLineEdit(settings.value("Database/Name").toString());
  DatabaseSettingsLayout->addWidget(DatabaseNameLineEdit, 2, 1, 1, 1);

  DatabaseUserNameLabel = new QLabel("Имя пользователя ");
  DatabaseSettingsLayout->addWidget(DatabaseUserNameLabel, 3, 0, 1, 1);

  DatabaseUserNameLineEdit =
      new QLineEdit(settings.value("Database/User/Name").toString());
  DatabaseSettingsLayout->addWidget(DatabaseUserNameLineEdit, 3, 1, 1, 1);

  DatabaseUserPasswordLabel = new QLabel("Пароль пользователя ");
  DatabaseSettingsLayout->addWidget(DatabaseUserPasswordLabel, 4, 0, 1, 1);

  DatabaseUserPasswordLineEdit =
      new QLineEdit(settings.value("Database/User/Password").toString());
  DatabaseSettingsLayout->addWidget(DatabaseUserPasswordLineEdit, 4, 1, 1, 1);

  // Кнопка сохранения настроек
  ApplySettingsPushButton = new QPushButton("Применить изменения");
  SettingsMainSubLayout->addWidget(ApplySettingsPushButton);

  // Сжимаем позиционирование
  SettingsVerticalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  SettingsMainSubLayout->addItem(SettingsVerticalSpacer1);
}

void MasterGUI::on_FullPersonalizationCheckBoxChanged() {
  if (FullPersonalizationCheckBox->checkState() == Qt::Checked) {
    OrderCreationControlPanelSubWidget = new QWidget();
    OrderCreationControlPanelLayout->insertWidget(
        1, OrderCreationControlPanelSubWidget);

    OrderCreationControlPanelSubLayout = new QHBoxLayout();
    OrderCreationControlPanelSubWidget->setLayout(
        OrderCreationControlPanelSubLayout);

    PanFilePathLabel = new QLabel("PAN-файл");
    OrderCreationControlPanelSubLayout->addWidget(PanFilePathLabel);

    PanFilePathLineEdit = new QLineEdit();
    OrderCreationControlPanelSubLayout->addWidget(PanFilePathLineEdit);

    PanFileExplorePushButton = new QPushButton("Обзор");
    OrderCreationControlPanelSubLayout->addWidget(PanFileExplorePushButton);
    connect(PanFileExplorePushButton, &QPushButton::clicked, this,
            &MasterGUI::on_PanFileExplorePushButton_slot);
  } else {
    OrderCreationControlPanelLayout->removeWidget(
        OrderCreationControlPanelSubWidget);
    delete OrderCreationControlPanelSubWidget;
  }
}

void MasterGUI::on_PanFileExplorePushButton_slot() {
  QString filePath =
      QFileDialog::getOpenFileName(nullptr, "Выбрать файл", "./", "*.csv");
  if (!filePath.isEmpty()) {
    PanFilePathLineEdit->setText(filePath);
  }
}
