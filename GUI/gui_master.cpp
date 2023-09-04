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
  DatabaseRandomBufferView->resizeColumnsToContents();
  DatabaseRandomBufferView->update();

  OrderTableView->resizeColumnsToContents();
  OrderTableView->update();

  ProductionLineTableView->resizeColumnsToContents();
  ProductionLineTableView->update();
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

  DatabaseControlPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DatabaseControlPanelLayout->addItem(DatabaseControlPanelVS);

  DatabaseTableChoice = new QComboBox();
  DatabaseTableChoice->addItem("production_lines");
  DatabaseTableChoice->addItem("transponders");
  DatabaseTableChoice->addItem("orders");
  DatabaseTableChoice->addItem("issuers");
  DatabaseTableChoice->addItem("boxes");
  DatabaseTableChoice->addItem("pallets");
  DatabaseTableChoice->addItem("transport_master_keys");
  DatabaseTableChoice->addItem("commercial_master_keys");
  DatabaseControlPanelLayout->addWidget(DatabaseTableChoice);

  ShowDatabaseTablePushButton = new QPushButton("Открыть таблицу");
  DatabaseControlPanelLayout->addWidget(ShowDatabaseTablePushButton);
  ClearDatabaseTablePushButton = new QPushButton("Очистить таблицу");
  DatabaseControlPanelLayout->addWidget(ClearDatabaseTablePushButton);
  InitIssuerTablePushButton =
      new QPushButton("Инициализация таблицы эмитентов");
  DatabaseControlPanelLayout->addWidget(InitIssuerTablePushButton);

  DatabaseControlPanelVS1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  DatabaseControlPanelLayout->addItem(DatabaseControlPanelVS1);

  CustomRequestLineEdit = new QLineEdit();
  DatabaseControlPanelLayout->addWidget(CustomRequestLineEdit);

  TransmitCustomRequestPushButton = new QPushButton("Отправить команду");
  DatabaseControlPanelLayout->addWidget(TransmitCustomRequestPushButton);

  // Отображение буфера считанных данных из БД
  DatabaseBufferGroup = new QGroupBox(QString("Буффер считанных данных"));
  DatabaseBufferGroup->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  DatabaseMainLayout->addWidget(DatabaseBufferGroup);

  DatabaseBufferLayout = new QVBoxLayout();
  DatabaseBufferGroup->setLayout(DatabaseBufferLayout);

  DatabaseRandomBufferView = new QTableView();
  DatabaseBufferLayout->addWidget(DatabaseRandomBufferView);

  // Настройка пропорции между объектами на макете
  DatabaseMainLayout->setStretch(0, 1);
  DatabaseMainLayout->setStretch(1, 3);
}

void MasterGUI::createOrderTab() {
  OrderTab = new QWidget();
  Tabs->addTab(OrderTab, "Создание заказов");

  // Основной макет
  OrderTabMainLayout = new QHBoxLayout();
  OrderTab->setLayout(OrderTabMainLayout);

  // Панель для создания
  OrderControlPanel = new QGroupBox("Панель управления");
  OrderTabMainLayout->addWidget(OrderControlPanel);

  OrderControlPanelLayout = new QVBoxLayout();
  OrderControlPanel->setLayout(OrderControlPanelLayout);

  FullPersonalizationCheckBox = new QCheckBox("Полная персонализация");
  OrderControlPanelLayout->addWidget(FullPersonalizationCheckBox);
  connect(FullPersonalizationCheckBox, &QCheckBox::stateChanged, this,
          &MasterGUI::on_FullPersonalizationCheckBoxChanged_slot);

  OrderPanelSubLayout1 = new QHBoxLayout();
  OrderControlPanelLayout->addLayout(OrderPanelSubLayout1);

  IssuerNameComboLabel = new QLabel("Компания заказчик");
  OrderPanelSubLayout1->addWidget(IssuerNameComboLabel);

  IssuerNameComboBox = new QComboBox();
  IssuerNameComboBox->addItem("Новое качество дорог");
  IssuerNameComboBox->addItem("Западный скоростной диаметр");
  OrderPanelSubLayout1->addWidget(IssuerNameComboBox);

  OrderPanelSubLayout2 = new QHBoxLayout();
  OrderControlPanelLayout->addLayout(OrderPanelSubLayout2);
  TransponderQuantityLabel = new QLabel("Количество транспондеров");
  OrderPanelSubLayout2->addWidget(TransponderQuantityLabel);
  TransponderQuantityLineEdit = new QLineEdit("1000");
  OrderPanelSubLayout2->addWidget(TransponderQuantityLineEdit);

  OrderPanelSubLayout3 = new QHBoxLayout();
  OrderControlPanelLayout->addLayout(OrderPanelSubLayout3);
  BoxCapacityLabel = new QLabel("Емкость бокса");
  OrderPanelSubLayout3->addWidget(BoxCapacityLabel);
  BoxCapacityLineEdit = new QLineEdit("50");
  OrderPanelSubLayout3->addWidget(BoxCapacityLineEdit);

  OrderPanelSubLayout4 = new QHBoxLayout();
  OrderControlPanelLayout->addLayout(OrderPanelSubLayout4);
  PalletCapacityLabel = new QLabel("Емкость палеты");
  OrderPanelSubLayout4->addWidget(PalletCapacityLabel);
  PalletCapacityLineEdit = new QLineEdit("10");
  OrderPanelSubLayout4->addWidget(PalletCapacityLineEdit);

  CreateNewOrderPushButton = new QPushButton("Создать новый заказ");
  OrderControlPanelLayout->addWidget(CreateNewOrderPushButton);

  OrderControlPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  OrderControlPanelLayout->addItem(OrderControlPanelVS);

  UpdateOrderViewPushButton = new QPushButton("Обновить таблицу");
  OrderControlPanelLayout->addWidget(UpdateOrderViewPushButton);

  DeleteLastOrderPushButton =
      new QPushButton("Удалить последний созданный заказ");
  OrderControlPanelLayout->addWidget(DeleteLastOrderPushButton);

  // Панель для отображения таблицы заказов
  OrderTablePanel = new QGroupBox("Таблица заказов");
  OrderTabMainLayout->addWidget(OrderTablePanel);

  OrderTablePanelLayout = new QVBoxLayout();
  OrderTablePanel->setLayout(OrderTablePanelLayout);

  OrderTableView = new QTableView();
  OrderTablePanelLayout->addWidget(OrderTableView);

  // Настройка пропорции между объектами на макете
  OrderTabMainLayout->setStretch(0, 1);
  OrderTabMainLayout->setStretch(1, 3);
}

void MasterGUI::createProductionLineTab() {
  ProductionLinesTab = new QWidget();
  Tabs->addTab(ProductionLinesTab, "Линии производства");

  ProductionLinesTabMainLayout = new QHBoxLayout();
  ProductionLinesTab->setLayout(ProductionLinesTabMainLayout);

  // Панель управления
  ProductionLinesControlPanel = new QGroupBox("Панель управления");
  ProductionLinesTabMainLayout->addWidget(ProductionLinesControlPanel);

  ProductionLinesControlPanelLayout = new QVBoxLayout();
  ProductionLinesControlPanel->setLayout(ProductionLinesControlPanelLayout);

  LoginLayout = new QHBoxLayout();
  ProductionLinesControlPanelLayout->addLayout(LoginLayout);
  LoginLabel = new QLabel("Введите логин: ");
  LoginLayout->addWidget(LoginLabel);
  LoginLineEdit = new QLineEdit();
  LoginLayout->addWidget(LoginLineEdit);

  PasswordLayout = new QHBoxLayout();
  ProductionLinesControlPanelLayout->addLayout(PasswordLayout);
  PasswordLabel = new QLabel("Введите пароль: ");
  PasswordLayout->addWidget(PasswordLabel);
  PasswordLineEdit = new QLineEdit();
  PasswordLayout->addWidget(PasswordLineEdit);

  CreateNewProductionLinePushButton =
      new QPushButton("Создать новую производственную линию");
  ProductionLinesControlPanelLayout->addWidget(
      CreateNewProductionLinePushButton);

  // Сжатие по вертикали
  ProductionLinesControlPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  ProductionLinesControlPanelLayout->addItem(ProductionLinesControlPanelVS);

  UpdateProductionLineViewPushButton = new QPushButton("Обновить таблицу");
  ProductionLinesControlPanelLayout->addWidget(
      UpdateProductionLineViewPushButton);

  DeleteLastProductionLinePushButton =
      new QPushButton("Удалить последнюю созданную линию производства");
  ProductionLinesControlPanelLayout->addWidget(
      DeleteLastProductionLinePushButton);

  // Панель для отображения таблицы производственных линий
  ProductionLineTablePanel = new QGroupBox("Таблица производственных линий");
  ProductionLinesTabMainLayout->addWidget(ProductionLineTablePanel);

  ProductionLineTableLayout = new QVBoxLayout();
  ProductionLineTablePanel->setLayout(ProductionLineTableLayout);

  ProductionLineTableView = new QTableView();
  ProductionLineTableLayout->addWidget(ProductionLineTableView);

  // Настройка пропорции между объектами на основном макете
  ProductionLinesTabMainLayout->setStretch(0, 1);
  ProductionLinesTabMainLayout->setStretch(1, 3);
}

void MasterGUI::createTransponderTab() {
  TransponderTab = new QWidget();
  Tabs->addTab(TransponderTab, "Выпуск транспондеров");

  TransponderTabMainLayout = new QHBoxLayout();
  TransponderTab->setLayout(TransponderTabMainLayout);

  // Панель управления
  TransponderControlPanel = new QGroupBox("Панель управления");
  TransponderTabMainLayout->addWidget(TransponderControlPanel);

  TransponderControlPanelLayout = new QVBoxLayout();
  TransponderControlPanel->setLayout(TransponderControlPanelLayout);

  UcidLayout = new QHBoxLayout();
  TransponderControlPanelLayout->addLayout(UcidLayout);
  UcidLabel = new QLabel("Введите UCID: ");
  UcidLayout->addWidget(UcidLabel);
  UcidLineEdit = new QLineEdit();
  UcidLayout->addWidget(UcidLineEdit);

  ReleaseTransponderPushButton = new QPushButton("Выпуск");
  TransponderControlPanelLayout->addWidget(ReleaseTransponderPushButton);

  TransponderControlPanelVS =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  TransponderControlPanelLayout->addItem(TransponderControlPanelVS);

  SearchByLayout = new QHBoxLayout();
  TransponderControlPanelLayout->addLayout(SearchByLayout);
  SearchByLabel = new QLabel("Поиск по: ");
  SearchByLayout->addWidget(SearchByLabel);
  SearchByComboBox = new QComboBox();
  SearchByComboBox->addItem("UCID");
  SearchByComboBox->addItem("SN");
  SearchByComboBox->addItem("PAN");
  SearchByComboBox->setCurrentIndex(0);
  SearchByLayout->addWidget(SearchByComboBox);
  connect(SearchByComboBox, &QComboBox::currentTextChanged, this,
          &MasterGUI::on_SearchByComboBox_slot);

  SearchInputLayout = new QHBoxLayout();
  TransponderControlPanelLayout->addLayout(SearchInputLayout);
  SearchInputLabel = new QLabel("Введите данные:");
  SearchInputLayout->addWidget(SearchInputLabel);
  SearchInputLineEdit = new QLineEdit();
  SearchInputLineEdit->setMaxLength(UCID_LENGTH);
  SearchInputLayout->addWidget(SearchInputLineEdit);

  SearchPushButton = new QPushButton("Найти");
  TransponderControlPanelLayout->addWidget(SearchPushButton);
  RereleaseTransponderPushButton = new QPushButton("Перевыпустить");
  TransponderControlPanelLayout->addWidget(RereleaseTransponderPushButton);
  RevokeTransponderPushButton = new QPushButton("Отозвать");
  TransponderControlPanelLayout->addWidget(RevokeTransponderPushButton);

  // Панель отображения
  TransponderDisplayPanel = new QGroupBox("Данные транспондера");
  TransponderTabMainLayout->addWidget(TransponderDisplayPanel);

  TransponderDisplayLayout = new QHBoxLayout();
  TransponderDisplayPanel->setLayout(TransponderDisplayLayout);

  TransponderDataListView = new QListView();
  TransponderDisplayLayout->addWidget(TransponderDataListView);

  FirmwareDsrcDataView = new QPlainTextEdit();
  TransponderDisplayLayout->addWidget(FirmwareDsrcDataView);

  // Настройка пропорции между объектами на основном макете
  TransponderTabMainLayout->setStretch(0, 1);
  TransponderTabMainLayout->setStretch(1, 3);
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
  TransportKeyControlPanelLayout->addItem(TransportKeyVS1);

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
  CommercialKeyControlPanelLayout->addItem(CommercialKeyVS1);

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

  DatabaseLogOptionLabel = new QLabel("Логирование ");
  DatabaseSettingsLayout->addWidget(DatabaseLogOptionLabel, 5, 0, 1, 1);

  DatabaseLogOption = new QCheckBox();
  DatabaseLogOption->setCheckState(
      settings.value("Database/Log/Active").toBool() ? Qt::Checked
                                                     : Qt::Unchecked);
  DatabaseSettingsLayout->addWidget(DatabaseLogOption, 5, 1, 1, 1);

  // Настройки генератора прошивок
  FirmwareSettingsGroupBox = new QGroupBox("Настройки генератора прошивок");
  SettingsMainSubLayout->addWidget(FirmwareSettingsGroupBox);

  FirmwareSettingsLayout = new QGridLayout();
  FirmwareSettingsGroupBox->setLayout(FirmwareSettingsLayout);

  FirmwareBasePathLabel = new QLabel("Путь к файлу с прошивкой");
  FirmwareSettingsLayout->addWidget(FirmwareBasePathLabel, 0, 0, 1, 1);
  FirmwareFileBaseLineEdit = new QLineEdit(
      settings.value("FirmwareGenerationSystem/Firmware/Path").toString());
  FirmwareFileBaseLineEdit->setMaxLength(200);
  FirmwareSettingsLayout->addWidget(FirmwareFileBaseLineEdit, 0, 1, 1, 1);
  ExploreFirmwareBasePathPushButton = new QPushButton("Обзор");
  FirmwareSettingsLayout->addWidget(ExploreFirmwareBasePathPushButton, 0, 2, 1,
                                    1);
  connect(ExploreFirmwareBasePathPushButton, &QPushButton::clicked, this,
          &MasterGUI::on_ExploreFirmwareBasePathPushButton_slot);

  FirmwareDataPathLabel = new QLabel("Путь к файлу с данными");
  FirmwareSettingsLayout->addWidget(FirmwareDataPathLabel, 1, 0, 1, 1);
  FirmwareDataPathLineEdit = new QLineEdit(
      settings.value("FirmwareGenerationSystem/TransponderData/Path")
          .toString());
  FirmwareDataPathLineEdit->setMaxLength(200);
  FirmwareSettingsLayout->addWidget(FirmwareDataPathLineEdit, 1, 1, 1, 1);
  ExploreFirmwareDataPathPushButton = new QPushButton("Обзор");
  FirmwareSettingsLayout->addWidget(ExploreFirmwareDataPathPushButton, 1, 2, 1,
                                    1);
  connect(ExploreFirmwareDataPathPushButton, &QPushButton::clicked, this,
          &MasterGUI::on_ExploreFirmwareDataPathPushButton_slot);

  // Кнопка сохранения настроек
  ApplySettingsPushButton = new QPushButton("Применить изменения");
  SettingsMainSubLayout->addWidget(ApplySettingsPushButton);

  // Сжатие по горизонтали
  SettingsVerticalSpacer1 =
      new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  SettingsMainSubLayout->addItem(SettingsVerticalSpacer1);
}

void MasterGUI::on_FullPersonalizationCheckBoxChanged_slot() {
  if (FullPersonalizationCheckBox->checkState() == Qt::Checked) {
    OrderPanelSubWidget = new QWidget();
    OrderControlPanelLayout->insertWidget(1, OrderPanelSubWidget);

    OrderPanelSubLayout = new QHBoxLayout();
    OrderPanelSubWidget->setLayout(OrderPanelSubLayout);

    PanFilePathLabel = new QLabel("PAN-файл");
    OrderPanelSubLayout->addWidget(PanFilePathLabel);

    PanFilePathLineEdit = new QLineEdit();
    OrderPanelSubLayout->addWidget(PanFilePathLineEdit);

    PanFileExplorePushButton = new QPushButton("Обзор");
    OrderPanelSubLayout->addWidget(PanFileExplorePushButton);
    connect(PanFileExplorePushButton, &QPushButton::clicked, this,
            &MasterGUI::on_PanFileExplorePushButton_slot);
  } else {
    OrderControlPanelLayout->removeWidget(OrderPanelSubWidget);
    delete OrderPanelSubWidget;
  }
}

void MasterGUI::on_PanFileExplorePushButton_slot() {
  QString filePath =
      QFileDialog::getOpenFileName(nullptr, "Выбрать файл", "./", "*.csv");
  if (!filePath.isEmpty()) {
    PanFilePathLineEdit->setText(filePath);
  }
}

void MasterGUI::on_SearchByComboBox_slot(const QString& text) {
  if (text == "UCID") {
    SearchInputLineEdit->setMaxLength(UCID_LENGTH);
  } else if (text == "PAN") {
    SearchInputLineEdit->setMaxLength(PAYMENT_MEANS_LENGTH);
  } else if (text == "SN") {
    SearchInputLineEdit->setMaxLength(SERIAL_NUMBER_LENGTH);
  } else {
    SearchInputLineEdit->setMaxLength(0);
  }
}

void MasterGUI::on_ExploreFirmwareBasePathPushButton_slot() {
  QString filePath =
      QFileDialog::getOpenFileName(this, "Выберите файл", "", "*.hex");
  FirmwareFileBaseLineEdit->setText(filePath);
}

void MasterGUI::on_ExploreFirmwareDataPathPushButton_slot() {
  QString filePath =
      QFileDialog::getOpenFileName(this, "Выберите файл", "", "*.hex");
  FirmwareDataPathLineEdit->setText(filePath);
}
