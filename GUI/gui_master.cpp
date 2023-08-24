#include "gui_master.h"

MasterGUI::MasterGUI(QWidget* parent) : GUI(parent, Master) {
  setObjectName("MasterGUI");
}

void MasterGUI::create() {
  // Создаем вкладки мастер меню
  createTabs();

  // Создаем виджеты логов
  createLog();

  // Настраиваем пропорции отображаемых элементов
  MainLayout->setStretch(0, 2);
  MainLayout->setStretch(1, 1);
}

void MasterGUI::update() {
  DatabaseBufferView->resizeColumnsToContents();
  DatabaseBufferView->update();

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

  // Контруируем вкладки для взаимодействия с базой данных
  createDatabaseTab();
  createOrderTab();
  createProductionLineTab();
  createTransponderTab();
  createIssuerTab();
  createBoxTab();
  createPalletTab();
  createTransportKeyTab();
  createCommercialKeyTab();

  // Конструируем вкладку настроек
  createSettingsTab();
}

void MasterGUI::createServerTab() {}

void MasterGUI::createDatabaseTab() {
  DatabaseTab = new QWidget();
  Tabs->addTab(DatabaseTab, "База данных");

  // Основной макет
  DatabaseMainLayout = new QHBoxLayout();
  DatabaseTab->setLayout(DatabaseMainLayout);

  // Панель управления
  DatabaseControlPanelGroup = new QGroupBox(QString("Панель управления"));
  DatabaseControlPanelGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  DatabaseMainLayout->addWidget(DatabaseControlPanelGroup);

  DatabaseControlPanelLayout = new QVBoxLayout();
  DatabaseControlPanelGroup->setLayout(DatabaseControlPanelLayout);

  // Кнопки
  ConnectDatabasePushButton = new QPushButton("Подключиться");
  DatabaseControlPanelLayout->addWidget(ConnectDatabasePushButton);

  DisconnectDatabasePushButton = new QPushButton("Отключиться");
  DatabaseControlPanelLayout->addWidget(DisconnectDatabasePushButton);

  PushButtonLayoutVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DatabaseControlPanelLayout->addItem(PushButtonLayoutVS1);

  CustomRequestLineEdit = new QLineEdit();
  DatabaseControlPanelLayout->addWidget(CustomRequestLineEdit);

  TransmitCustomRequestPushButton = new QPushButton("Отправить команду");
  DatabaseControlPanelLayout->addWidget(TransmitCustomRequestPushButton);

  // Отображение буфера считанных данных из БД
  DatabaseBufferGroup = new QGroupBox(QString("Буффер считанных данных"));
  DatabaseBufferGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  DatabaseMainLayout->addWidget(DatabaseBufferGroup);

  DatabaseControlPanelLayout = new QVBoxLayout();
  DatabaseBufferGroup->setLayout(DatabaseControlPanelLayout);

  DatabaseBufferView = new QTableView();
  DatabaseControlPanelLayout->addWidget(DatabaseBufferView);

  // Настройка пропорции между объектами на макете
  DatabaseMainLayout->setStretch(0, 1);
  DatabaseMainLayout->setStretch(1, 3);
}

void MasterGUI::createProductionLineTab() {
  ProductionLineTab = new QWidget();
  Tabs->addTab(ProductionLineTab, "Линии производства");

  // Основной макет
  ProductionLineMainLayout = new QHBoxLayout();
  ProductionLineTab->setLayout(ProductionLineMainLayout);

  // Панель управления
  ProductionLineControlPanelGroup = new QGroupBox(QString("Панель управления"));
  ProductionLineControlPanelGroup->setAlignment(Qt::AlignHCenter |
                                                Qt::AlignVCenter);
  ProductionLineMainLayout->addWidget(ProductionLineControlPanelGroup);

  ProductionLineControlPanelLayout = new QVBoxLayout();
  ProductionLineControlPanelGroup->setLayout(ProductionLineControlPanelLayout);

  // Кнопки
  UpdateProductionLinePushButton = new QPushButton("Обновить");
  ProductionLineControlPanelLayout->addWidget(UpdateProductionLinePushButton);

  ProductionLineVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  ProductionLineControlPanelLayout->addItem(ProductionLineVS1);

  ClearProductionLinePushButton = new QPushButton("Очистить");
  ProductionLineControlPanelLayout->addWidget(ClearProductionLinePushButton);

  // Отображение буфера считанных данных из БД
  ProductionLineViewGroup = new QGroupBox(QString("Таблица"));
  ProductionLineViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  ProductionLineMainLayout->addWidget(ProductionLineViewGroup);

  ProductionLineViewLayout = new QVBoxLayout();
  ProductionLineViewGroup->setLayout(ProductionLineViewLayout);

  ProductionLineView = new QTableView();
  ProductionLineViewLayout->addWidget(ProductionLineView);

  // Настройка пропорции между объектами на макете
  ProductionLineMainLayout->setStretch(0, 1);
  ProductionLineMainLayout->setStretch(1, 3);
}

void MasterGUI::createTransponderTab() {
  TransponderTab = new QWidget();
  Tabs->addTab(TransponderTab, "Транспондеры");

  // Основной макет
  TransponderMainLayout = new QHBoxLayout();
  TransponderTab->setLayout(TransponderMainLayout);

  // Панель управления
  TransponderControlPanelGroup = new QGroupBox(QString("Панель управления"));
  TransponderControlPanelGroup->setAlignment(Qt::AlignHCenter |
                                             Qt::AlignVCenter);
  TransponderMainLayout->addWidget(TransponderControlPanelGroup);

  TransponderControlPanelLayout = new QVBoxLayout();
  TransponderControlPanelGroup->setLayout(TransponderControlPanelLayout);

  // Кнопки
  UpdateTransponderPushButton = new QPushButton("Обновить");
  TransponderControlPanelLayout->addWidget(UpdateTransponderPushButton);

  TransponderVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  TransponderControlPanelLayout->addItem(ProductionLineVS1);

  ClearTransponderPushButton = new QPushButton("Очистить");
  TransponderControlPanelLayout->addWidget(ClearTransponderPushButton);

  // Отображение буфера считанных данных из БД
  TransponderViewGroup = new QGroupBox(QString("Таблица"));
  TransponderViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  TransponderMainLayout->addWidget(TransponderViewGroup);

  TransponderViewLayout = new QVBoxLayout();
  TransponderViewGroup->setLayout(TransponderViewLayout);

  TransponderView = new QTableView();
  TransponderViewLayout->addWidget(TransponderView);

  // Настройка пропорции между объектами на макете
  TransponderMainLayout->setStretch(0, 1);
  TransponderMainLayout->setStretch(1, 3);
}

void MasterGUI::createOrderTab() {
  OrderTab = new QWidget();
  Tabs->addTab(OrderTab, "Создание заказов");

  // Основной макет
  OrderTabMainLayout = new QHBoxLayout();
  OrderTab->setLayout(OrderTabMainLayout);

  // Макет для панелей
  OrderTabPanelLayout = new QVBoxLayout();
  OrderTabMainLayout->addLayout(OrderTabPanelLayout);

  // Панель управления
  OrderControlPanel = new QGroupBox("Панель управления");
  OrderTabPanelLayout->addWidget(OrderControlPanel);

  OrderControlPanelLayout = new QVBoxLayout();
  OrderControlPanel->setLayout(OrderControlPanelLayout);

  UpdateOrderPushButton = new QPushButton("Обновить");
  OrderControlPanelLayout->addWidget(UpdateOrderPushButton);

  OrderControlPanelVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  OrderControlPanelLayout->addItem(OrderControlPanelVS1);

  ClearOrderPushButton = new QPushButton("Очистить");
  OrderControlPanelLayout->addWidget(ClearOrderPushButton);

  // Панель для создания
  OrderCreationPanel = new QGroupBox("Панель создания");
  OrderTabPanelLayout->addWidget(OrderCreationPanel);

  OrderCreationPanelLayout = new QVBoxLayout();
  OrderCreationPanel->setLayout(OrderCreationPanelLayout);

  FullPersonalizationCheckBox = new QCheckBox("Полная персонализация");
  OrderCreationPanelLayout->addWidget(FullPersonalizationCheckBox);
  connect(FullPersonalizationCheckBox, &QCheckBox::stateChanged, this,
          &MasterGUI::on_FullPersonalizationCheckBoxChanged);

  OrderCreationPanelSubLayout1 = new QHBoxLayout();
  OrderCreationPanelLayout->addLayout(OrderCreationPanelSubLayout1);

  IssuerNameComboLabel = new QLabel("Компания заказчик");
  OrderCreationPanelSubLayout1->addWidget(IssuerNameComboLabel);

  IssuerNameComboBox = new QComboBox();
  IssuerNameComboBox->addItem("Новое качество дорог");
  IssuerNameComboBox->addItem("Западный скоростной диаметр");
  OrderCreationPanelSubLayout1->addWidget(IssuerNameComboBox);

  OrderCreationPanelSubLayout2 = new QHBoxLayout();
  OrderCreationPanelLayout->addLayout(OrderCreationPanelSubLayout2);

  TransponderQuantityLabel = new QLabel("Количество транспондеров");
  OrderCreationPanelSubLayout2->addWidget(TransponderQuantityLabel);

  TransponderQuantityLineEdit = new QLineEdit("100");
  OrderCreationPanelSubLayout2->addWidget(TransponderQuantityLineEdit);

  CreateNewOrderPushButton = new QPushButton("Создать новый заказ");
  OrderCreationPanelLayout->addWidget(CreateNewOrderPushButton);

  OrderCreationPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  OrderCreationPanelLayout->addItem(OrderCreationPanelVS);

  // Панель управления инициализацией транспондеров
  PanListPanel = new QGroupBox("Список новых PAN");
  OrderTabMainLayout->addWidget(PanListPanel);

  PanListLayout = new QVBoxLayout();
  PanListPanel->setLayout(PanListLayout);

  PanListTableView = new QTableView();
  PanListLayout->addWidget(PanListTableView);

  // Сжатие по горизонтали
  OrderTabMainLayoutHS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  OrderTabMainLayout->addItem(OrderTabMainLayoutHS);

  // Настройка пропорции между объектами на макете
  OrderTabMainLayout->setStretch(0, 1);
  OrderTabMainLayout->setStretch(1, 3);
}

void MasterGUI::createIssuerTab() {
  IssuerTab = new QWidget();
  Tabs->addTab(IssuerTab, "Эмитенты");

  // Основной макет
  IssuerMainLayout = new QHBoxLayout();
  IssuerTab->setLayout(IssuerMainLayout);

  // Панель управления
  IssuerControlPanelGroup = new QGroupBox(QString("Панель управления"));
  IssuerControlPanelGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  IssuerMainLayout->addWidget(IssuerControlPanelGroup);

  IssuerControlPanelLayout = new QVBoxLayout();
  IssuerControlPanelGroup->setLayout(IssuerControlPanelLayout);

  // Кнопки
  UpdateIssuerPushButton = new QPushButton("Обновить");
  IssuerControlPanelLayout->addWidget(UpdateIssuerPushButton);

  IssuerVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  IssuerControlPanelLayout->addItem(ProductionLineVS1);

  ClearIssuerPushButton = new QPushButton("Очистить");
  IssuerControlPanelLayout->addWidget(ClearIssuerPushButton);

  // Отображение буфера считанных данных из БД
  IssuerViewGroup = new QGroupBox(QString("Таблица"));
  IssuerViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  IssuerMainLayout->addWidget(IssuerViewGroup);

  IssuerViewLayout = new QVBoxLayout();
  IssuerViewGroup->setLayout(IssuerViewLayout);

  IssuerView = new QTableView();
  IssuerViewLayout->addWidget(IssuerView);

  // Настройка пропорции между объектами на макете
  IssuerMainLayout->setStretch(0, 1);
  IssuerMainLayout->setStretch(1, 3);
}

void MasterGUI::createBoxTab() {
  BoxTab = new QWidget();
  Tabs->addTab(BoxTab, "Боксы");

  // Основной макет
  BoxMainLayout = new QHBoxLayout();
  BoxTab->setLayout(BoxMainLayout);

  // Панель управления
  BoxControlPanelGroup = new QGroupBox(QString("Панель управления"));
  BoxControlPanelGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  BoxMainLayout->addWidget(BoxControlPanelGroup);

  BoxControlPanelLayout = new QVBoxLayout();
  BoxControlPanelGroup->setLayout(BoxControlPanelLayout);

  // Кнопки
  UpdateBoxPushButton = new QPushButton("Обновить");
  BoxControlPanelLayout->addWidget(UpdateBoxPushButton);

  BoxVS1 = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  BoxControlPanelLayout->addItem(ProductionLineVS1);

  ClearBoxPushButton = new QPushButton("Очистить");
  BoxControlPanelLayout->addWidget(ClearBoxPushButton);

  // Отображение буфера считанных данных из БД
  BoxViewGroup = new QGroupBox(QString("Таблица"));
  BoxViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  BoxMainLayout->addWidget(BoxViewGroup);

  BoxViewLayout = new QVBoxLayout();
  BoxViewGroup->setLayout(BoxViewLayout);

  BoxView = new QTableView();
  BoxViewLayout->addWidget(BoxView);

  // Настройка пропорции между объектами на макете
  BoxMainLayout->setStretch(0, 1);
  BoxMainLayout->setStretch(1, 3);
}

void MasterGUI::createPalletTab() {
  PalletTab = new QWidget();
  Tabs->addTab(PalletTab, "Палеты");

  // Основной макет
  PalletMainLayout = new QHBoxLayout();
  PalletTab->setLayout(PalletMainLayout);

  // Панель управления
  PalletControlPanelGroup = new QGroupBox(QString("Панель управления"));
  PalletControlPanelGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  PalletMainLayout->addWidget(PalletControlPanelGroup);

  PalletControlPanelLayout = new QVBoxLayout();
  PalletControlPanelGroup->setLayout(PalletControlPanelLayout);

  // Кнопки
  UpdatePalletPushButton = new QPushButton("Обновить");
  PalletControlPanelLayout->addWidget(UpdatePalletPushButton);

  PalletVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  PalletControlPanelLayout->addItem(ProductionLineVS1);

  ClearPalletPushButton = new QPushButton("Очистить");
  PalletControlPanelLayout->addWidget(ClearPalletPushButton);

  // Отображение буфера считанных данных из БД
  PalletViewGroup = new QGroupBox(QString("Таблица"));
  PalletViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  PalletMainLayout->addWidget(PalletViewGroup);

  PalletViewLayout = new QVBoxLayout();
  PalletViewGroup->setLayout(PalletViewLayout);

  PalletView = new QTableView();
  PalletViewLayout->addWidget(PalletView);

  // Настройка пропорции между объектами на макете
  PalletMainLayout->setStretch(0, 1);
  PalletMainLayout->setStretch(1, 3);
}

void MasterGUI::createTransportKeyTab() {
  TransportKeyTab = new QWidget();
  Tabs->addTab(TransportKeyTab, "Транспортные ключи");

  // Основной макет
  TransportKeyMainLayout = new QHBoxLayout();
  TransportKeyTab->setLayout(TransportKeyMainLayout);

  // Панель управления
  TransportKeyControlPanelGroup = new QGroupBox(QString("Панель управления"));
  TransportKeyControlPanelGroup->setAlignment(Qt::AlignHCenter |
                                              Qt::AlignVCenter);
  TransportKeyMainLayout->addWidget(TransportKeyControlPanelGroup);

  TransportKeyControlPanelLayout = new QVBoxLayout();
  TransportKeyControlPanelGroup->setLayout(TransportKeyControlPanelLayout);

  // Кнопки
  UpdateTransportKeyPushButton = new QPushButton("Обновить");
  TransportKeyControlPanelLayout->addWidget(UpdateTransportKeyPushButton);

  TransportKeyVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  TransportKeyControlPanelLayout->addItem(ProductionLineVS1);

  ClearTransportKeyPushButton = new QPushButton("Очистить");
  TransportKeyControlPanelLayout->addWidget(ClearTransportKeyPushButton);

  // Отображение буфера считанных данных из БД
  TransportKeyViewGroup = new QGroupBox(QString("Таблица"));
  TransportKeyViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  TransportKeyMainLayout->addWidget(TransportKeyViewGroup);

  TransportKeyViewLayout = new QVBoxLayout();
  TransportKeyViewGroup->setLayout(TransportKeyViewLayout);

  TransportKeyView = new QTableView();
  TransportKeyViewLayout->addWidget(TransportKeyView);

  // Настройка пропорции между объектами на макете
  TransportKeyMainLayout->setStretch(0, 1);
  TransportKeyMainLayout->setStretch(1, 3);
}

void MasterGUI::createCommercialKeyTab(void) {
  CommercialKeyTab = new QWidget();
  Tabs->addTab(CommercialKeyTab, "Коммерческие ключи");

  // Основной макет
  CommercialKeyMainLayout = new QHBoxLayout();
  CommercialKeyTab->setLayout(CommercialKeyMainLayout);

  // Панель управления
  CommercialKeyControlPanelGroup = new QGroupBox(QString("Панель управления"));
  CommercialKeyControlPanelGroup->setAlignment(Qt::AlignHCenter |
                                               Qt::AlignVCenter);
  CommercialKeyMainLayout->addWidget(CommercialKeyControlPanelGroup);

  CommercialKeyControlPanelLayout = new QVBoxLayout();
  CommercialKeyControlPanelGroup->setLayout(CommercialKeyControlPanelLayout);

  // Кнопки
  UpdateCommercialKeyPushButton = new QPushButton("Обновить");
  CommercialKeyControlPanelLayout->addWidget(UpdateCommercialKeyPushButton);

  CommercialKeyVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  CommercialKeyControlPanelLayout->addItem(ProductionLineVS1);

  ClearCommercialKeyPushButton = new QPushButton("Очистить");
  CommercialKeyControlPanelLayout->addWidget(ClearCommercialKeyPushButton);

  // Отображение буфера считанных данных из БД
  CommercialKeyViewGroup = new QGroupBox(QString("Таблица"));
  CommercialKeyViewGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  CommercialKeyMainLayout->addWidget(CommercialKeyViewGroup);

  CommercialKeyViewLayout = new QVBoxLayout();
  CommercialKeyViewGroup->setLayout(CommercialKeyViewLayout);

  CommercialKeyView = new QTableView();
  CommercialKeyViewLayout->addWidget(CommercialKeyView);

  // Настройка пропорции между объектами на макете
  CommercialKeyMainLayout->setStretch(0, 1);
  CommercialKeyMainLayout->setStretch(1, 3);
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
    OrderCreationPanelSubWidget = new QWidget();
    OrderCreationPanelLayout->insertWidget(1, OrderCreationPanelSubWidget);

    OrderCreationPanelSubLayout = new QHBoxLayout();
    OrderCreationPanelSubWidget->setLayout(OrderCreationPanelSubLayout);

    PanFilePathLabel = new QLabel("PAN-файл");
    OrderCreationPanelSubLayout->addWidget(PanFilePathLabel);

    PanFilePathLineEdit = new QLineEdit();
    OrderCreationPanelSubLayout->addWidget(PanFilePathLineEdit);

    PanFileExplorePushButton = new QPushButton("Обзор");
    OrderCreationPanelSubLayout->addWidget(PanFileExplorePushButton);
    connect(PanFileExplorePushButton, &QPushButton::clicked, this,
            &MasterGUI::on_PanFileExplorePushButton_slot);
  } else {
    OrderCreationPanelLayout->removeWidget(OrderCreationPanelSubWidget);
    delete OrderCreationPanelSubWidget;
  }
}

void MasterGUI::on_PanFileExplorePushButton_slot() {
  QString filePath =
      QFileDialog::getOpenFileName(nullptr, "Выбрать файл", "./", "*.csv");
  if (!filePath.isEmpty()) {
    PanFilePathLineEdit->setText(filePath);
  }
}
