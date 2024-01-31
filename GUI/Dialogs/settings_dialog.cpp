#include "settings_dialog.h"
#include "General/definitions.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
  setObjectName("SettingsDialog");

  // Считываем размеры дисплея
  DesktopGeometry = QApplication::primaryScreen()->size();

  // Создаем диалоговое окно
  setGeometry(DesktopGeometry.width() * 0.45, DesktopGeometry.height() * 0.45,
              DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1);
  setWindowTitle("Настройки");

  create();

  //  adjustSize();
  //  setFixedHeight(size().height());
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::create() {
  // Главный макет меню настроек
  MainLayout = new QVBoxLayout();
  setLayout(MainLayout);

  // Настройки базы данных
  DatabaseGroupBox = new QGroupBox(QString("Настройки базы данных"));
  MainLayout->addWidget(DatabaseGroupBox);

  DatabaseLayout = new QGridLayout();
  DatabaseGroupBox->setLayout(DatabaseLayout);

  DatabaseIpLabel = new QLabel("IP-адрес");
  DatabaseLayout->addWidget(DatabaseIpLabel, 0, 0, 1, 1);

  DatabaseIpLineEdit = new QLineEdit(
      Settings.value("postgre_sql_database/server_ip").toString());
  DatabaseLayout->addWidget(DatabaseIpLineEdit, 0, 1, 1, 1);

  DatabasePortLabel = new QLabel("Порт ");
  DatabaseLayout->addWidget(DatabasePortLabel, 1, 0, 1, 1);

  DatabasePortLineEdit = new QLineEdit(
      Settings.value("postgre_sql_database/server_port").toString());
  DatabaseLayout->addWidget(DatabasePortLineEdit, 1, 1, 1, 1);

  DatabaseNameLabel = new QLabel("Название базы данных ");
  DatabaseLayout->addWidget(DatabaseNameLabel, 2, 0, 1, 1);

  DatabaseNameLineEdit = new QLineEdit(
      Settings.value("postgre_sql_database/database_name").toString());
  DatabaseLayout->addWidget(DatabaseNameLineEdit, 2, 1, 1, 1);

  DatabaseUserNameLabel = new QLabel("Имя пользователя ");
  DatabaseLayout->addWidget(DatabaseUserNameLabel, 3, 0, 1, 1);

  DatabaseUserNameLineEdit = new QLineEdit(
      Settings.value("postgre_sql_database/user_name").toString());
  DatabaseLayout->addWidget(DatabaseUserNameLineEdit, 3, 1, 1, 1);

  DatabaseUserPasswordLabel = new QLabel("Пароль пользователя ");
  DatabaseLayout->addWidget(DatabaseUserPasswordLabel, 4, 0, 1, 1);

  DatabaseUserPasswordLineEdit = new QLineEdit(
      Settings.value("postgre_sql_database/user_password").toString());
  DatabaseLayout->addWidget(DatabaseUserPasswordLineEdit, 4, 1, 1, 1);

  // Настройки клиента
  PersoClientGroupBox = new QGroupBox(QString("Сетевые настройки"));
  MainLayout->addWidget(PersoClientGroupBox);

  PersoClientMainLayout = new QGridLayout();
  PersoClientGroupBox->setLayout(PersoClientMainLayout);

  PersoClientServerIdLabel =
      new QLabel("IP адрес или URL сервера персонализации");
  PersoClientMainLayout->addWidget(PersoClientServerIdLabel, 0, 0, 1, 1);
  PersoClientServerIpLineEdit =
      new QLineEdit(Settings.value("perso_client/server_ip").toString());
  PersoClientMainLayout->addWidget(PersoClientServerIpLineEdit, 0, 1, 1, 1);
  PersoClientServerPortLabel = new QLabel("Порт сервера персонализации");
  PersoClientMainLayout->addWidget(PersoClientServerPortLabel, 1, 0, 1, 1);
  PersoClientServerPortLineEdit =
      new QLineEdit(Settings.value("perso_client/server_port").toString());
  PersoClientMainLayout->addWidget(PersoClientServerPortLineEdit, 1, 1, 1, 1);

  // Настройки логгера
  LogSystemGroupBox = new QGroupBox("Настройки системы логгирования");
  MainLayout->addWidget(LogSystemGroupBox);

  LogSystemLayout = new QGridLayout();
  LogSystemGroupBox->setLayout(LogSystemLayout);

  LogSystemGlobalEnableLabel = new QLabel("Глобальное включение");
  LogSystemLayout->addWidget(LogSystemGlobalEnableLabel, 0, 0, 1, 1);
  LogSystemGlobalEnableCheckBox = new QCheckBox();
  LogSystemGlobalEnableCheckBox->setCheckState(
      Settings.value("log_system/global_enable").toBool() ? Qt::Checked
                                                          : Qt::Unchecked);
  LogSystemLayout->addWidget(LogSystemGlobalEnableCheckBox, 0, 1, 1, 1);
  connect(LogSystemGlobalEnableCheckBox, &QCheckBox::stateChanged, this,
          &SettingsDialog::logSystemEnableCheckBox_slot);

  LogSystemExtendedEnableLabel = new QLabel("Расширенное логгирование");
  LogSystemLayout->addWidget(LogSystemExtendedEnableLabel, 1, 0, 1, 1);
  LogSystemExtendedEnableCheckBox = new QCheckBox();
  LogSystemExtendedEnableCheckBox->setCheckState(
      Settings.value("log_system/extended_enable").toBool() ? Qt::Checked
                                                            : Qt::Unchecked);
  LogSystemLayout->addWidget(LogSystemExtendedEnableCheckBox, 1, 1, 1, 1);

  LogSystemProxyWidget1 = new QWidget();
  LogSystemLayout->addWidget(LogSystemProxyWidget1, 2, 0, 1, 2);
  if (!LogSystemGlobalEnableCheckBox->isChecked()) {
    LogSystemProxyWidget1->hide();
  }
  LogSystemProxyWidget1Layout = new QGridLayout();
  LogSystemProxyWidget1->setLayout(LogSystemProxyWidget1Layout);

  LogSystemDisplayEnableLabel = new QLabel("Вывод на дисплей вкл/выкл");
  LogSystemProxyWidget1Layout->addWidget(LogSystemDisplayEnableLabel, 0, 0, 1,
                                         1);
  LogSystemDisplayEnableCheckBox = new QCheckBox();
  LogSystemDisplayEnableCheckBox->setCheckState(
      Settings.value("log_system/display_log_enable").toBool() ? Qt::Checked
                                                               : Qt::Unchecked);
  LogSystemProxyWidget1Layout->addWidget(LogSystemDisplayEnableCheckBox, 0, 1,
                                         1, 1);

  LogSystemListenPersoServerLabel =
      new QLabel("Получение логов с сервера персонализации");
  LogSystemProxyWidget1Layout->addWidget(LogSystemListenPersoServerLabel, 1, 0,
                                         1, 1);
  LogSystemListenPersoServerCheckBox = new QCheckBox();
  LogSystemListenPersoServerCheckBox->setCheckState(
      Settings.value("log_system/udp_listen_enable").toBool() ? Qt::Checked
                                                              : Qt::Unchecked);
  LogSystemProxyWidget1Layout->addWidget(LogSystemListenPersoServerCheckBox, 1,
                                         1, 1, 1);
  connect(LogSystemListenPersoServerCheckBox, &QCheckBox::stateChanged, this,
          &SettingsDialog::logSystemListenPersoServerCheckBox_slot);

  LogSystemProxyWidget2 = new QWidget();
  LogSystemProxyWidget1Layout->addWidget(LogSystemProxyWidget2, 3, 0, 1, 2);
  if (!LogSystemListenPersoServerCheckBox->isChecked()) {
    LogSystemProxyWidget2->hide();
  }
  LogSystemProxyWidget2Layout = new QGridLayout();
  LogSystemProxyWidget2->setLayout(LogSystemProxyWidget2Layout);

  LogSystemListenIpLabel = new QLabel("Прослушиваемый IP");
  LogSystemProxyWidget2Layout->addWidget(LogSystemListenIpLabel, 0, 0, 1, 1);
  LogSystemListenIpLineEdit =
      new QLineEdit(Settings.value("log_system/udp_listen_ip").toString());
  LogSystemListenIpLineEdit->setMaxLength(300);
  LogSystemProxyWidget2Layout->addWidget(LogSystemListenIpLineEdit, 0, 1, 1, 1);

  LogSystemListenPortLabel = new QLabel("Прослушиваемый порт");
  LogSystemProxyWidget2Layout->addWidget(LogSystemListenPortLabel, 1, 0, 1, 1);
  LogSystemListenPortLineEdit =
      new QLineEdit(Settings.value("log_system/udp_listen_port").toString());
  LogSystemProxyWidget2Layout->addWidget(LogSystemListenPortLineEdit, 1, 1, 1,
                                         1);

  LogSystemFileEnableLabel = new QLabel("Логгирование в файл");
  LogSystemProxyWidget1Layout->addWidget(LogSystemFileEnableLabel, 4, 0, 1, 1);
  LogSystemFileEnableCheckBox = new QCheckBox();
  LogSystemFileEnableCheckBox->setCheckState(
      Settings.value("log_system/file_log_enable").toBool() ? Qt::Checked
                                                            : Qt::Unchecked);
  LogSystemProxyWidget1Layout->addWidget(LogSystemFileEnableCheckBox, 4, 1, 1,
                                         1);
  connect(LogSystemFileEnableCheckBox, &QCheckBox::stateChanged, this,
          &SettingsDialog::logSystemFileEnableCheckBox_slot);

  LogSystemProxyWidget3 = new QWidget();
  LogSystemProxyWidget1Layout->addWidget(LogSystemProxyWidget3, 5, 0, 1, 2);
  if (!LogSystemFileEnableCheckBox->isChecked()) {
    LogSystemProxyWidget3->hide();
  }
  LogSystemProxyWidget3Layout = new QGridLayout();
  LogSystemProxyWidget3->setLayout(LogSystemProxyWidget3Layout);

  LogSystemFileMaxNumberLabel =
      new QLabel("Максимальное количество лог-файлов");
  LogSystemProxyWidget3Layout->addWidget(LogSystemFileMaxNumberLabel, 0, 0, 1,
                                         1);
  LogSystemFileMaxNumberLineEdit = new QLineEdit(
      Settings.value("log_system/log_file_max_number").toString());
  LogSystemFileMaxNumberLineEdit->setMaxLength(10);
  LogSystemProxyWidget3Layout->addWidget(LogSystemFileMaxNumberLineEdit, 0, 1,
                                         1, 1);

  // Настройки принтера стикеров
  StickerPrinterGroupBox = new QGroupBox(QString("Стикер-принтер"));
  MainLayout->addWidget(StickerPrinterGroupBox);

  StickerPrinterMainLayout = new QGridLayout();
  StickerPrinterGroupBox->setLayout(StickerPrinterMainLayout);

  StickerPrinterLibPathLabel = new QLabel("Путь к библиотеке");
  StickerPrinterMainLayout->addWidget(StickerPrinterLibPathLabel, 0, 0, 1, 1);
  StickerPrinterLibPathLineEdit =
      new QLineEdit(Settings.value("sticker_printer/library_path").toString());
  StickerPrinterMainLayout->addWidget(StickerPrinterLibPathLineEdit, 0, 1, 1,
                                      1);
  StickerPrinterLibPathPushButton = new QPushButton("Обзор");
  StickerPrinterMainLayout->addWidget(StickerPrinterLibPathPushButton, 0, 2, 1,
                                      1);
  connect(StickerPrinterLibPathPushButton, &QPushButton::clicked, this,
          &SettingsDialog::stickerPrinterLibPathPushButton_slot);

  StickerPrinterNameLabel = new QLabel("Название");
  StickerPrinterMainLayout->addWidget(StickerPrinterNameLabel, 1, 0, 1, 1);
  StickerPrinterNameLineEdit =
      new QLineEdit(Settings.value("sticker_printer/name").toString());
  StickerPrinterMainLayout->addWidget(StickerPrinterNameLineEdit, 1, 1, 1, 2);

  // Кнопки
  ButtonLayout = new QHBoxLayout();
  MainLayout->addLayout(ButtonLayout);

  ApplyPushButton = new QPushButton("Применить");
  ButtonLayout->addWidget(ApplyPushButton);
  connect(ApplyPushButton, &QPushButton::clicked, this,
          &SettingsDialog::accept);

  RejectPushButton = new QPushButton("Закрыть");
  ButtonLayout->addWidget(RejectPushButton);
  connect(RejectPushButton, &QPushButton::clicked, this, &QDialog::reject);
}

bool SettingsDialog::check() const {
  QFileInfo info;
  QHostAddress ip;
  int32_t port;

  if (LogSystemGlobalEnableCheckBox->checkState() == Qt::Unchecked) {
    return true;
  }

  if (LogSystemListenPersoServerCheckBox->checkState() == Qt::Checked) {
    ip = QHostAddress(LogSystemListenIpLineEdit->text());
    if (ip.isNull()) {
      return false;
    }

    port = LogSystemListenPortLineEdit->text().toInt();
    if ((port > IP_PORT_MAX_VALUE) || (port < IP_PORT_MIN_VALUE)) {
      return false;
    }
  }

  ip = QHostAddress(DatabaseIpLineEdit->text());
  if (ip.isNull()) {
    return false;
  }

  ip = QHostAddress(PersoClientServerIpLineEdit->text());
  if (ip.isNull()) {
    return false;
  }

  port = PersoClientServerPortLineEdit->text().toInt();
  if ((port > IP_PORT_MAX_VALUE) || (port < IP_PORT_MIN_VALUE)) {
    return false;
  }

  if (LogSystemFileEnableCheckBox->checkState() == Qt::Checked) {
    port = DatabasePortLineEdit->text().toInt();
    if ((port > IP_PORT_MAX_VALUE) || (port < IP_PORT_MIN_VALUE)) {
      return false;
    }

    if (LogSystemFileMaxNumberLineEdit->text().toInt() == 0) {
      return false;
    }
  }

  info.setFile(StickerPrinterLibPathLineEdit->text());
  if (!info.isFile()) {
    return false;
  }

  return true;
}

void SettingsDialog::save() {
  // Настройки системы логгирования
  Settings.setValue("log_system/global_enable",
                    LogSystemGlobalEnableCheckBox->checkState() == Qt::Checked
                        ? true
                        : false);
  Settings.setValue("log_system/extended_enable",
                    LogSystemExtendedEnableCheckBox->checkState() == Qt::Checked
                        ? true
                        : false);
  Settings.setValue("log_system/display_log_enable",
                    LogSystemDisplayEnableCheckBox->checkState() == Qt::Checked
                        ? true
                        : false);
  Settings.setValue(
      "log_system/file_log_enable",
      LogSystemFileEnableCheckBox->checkState() == Qt::Checked ? true : false);
  Settings.setValue("log_system/log_file_max_number",
                    LogSystemFileMaxNumberLineEdit->text());
  Settings.setValue(
      "log_system/udp_listen_enable",
      LogSystemListenPersoServerCheckBox->checkState() == Qt::Checked ? true
                                                                      : false);
  Settings.setValue("log_system/udp_listen_ip",
                    LogSystemListenIpLineEdit->text());
  Settings.setValue("log_system/udp_listen_port",
                    LogSystemListenPortLineEdit->text().toInt());

  // Настройки клиента
  Settings.setValue("perso_client/server_ip",
                    PersoClientServerIpLineEdit->text());
  Settings.setValue("perso_client/server_port",
                    PersoClientServerPortLineEdit->text());

  // Настройки контроллера базы данных
  Settings.setValue("postgre_sql_database/server_ip",
                    DatabaseIpLineEdit->text());
  Settings.setValue("postgre_sql_database/server_port",
                    DatabasePortLineEdit->text().toInt());
  Settings.setValue("postgre_sql_database/database_name",
                    DatabaseNameLineEdit->text());
  Settings.setValue("postgre_sql_database/user_name",
                    DatabaseUserNameLineEdit->text());
  Settings.setValue("postgre_sql_database/user_password",
                    DatabaseUserPasswordLineEdit->text());

  // Принтер стикеров
  Settings.setValue("sticker_printer/library_path",
                    StickerPrinterLibPathLineEdit->text());
  Settings.setValue("sticker_printer/name", StickerPrinterNameLineEdit->text());
}

void SettingsDialog::accept() {
  if (!check()) {
    QMessageBox::critical(this, "Ошибка", "Некорректный ввод данных.",
                          QMessageBox::Ok);
    return;
  }

  save();
  QMessageBox::information(this, "Оповещение", "Новые настройки применены.",
                           QMessageBox::Ok);
  emit applyNewSettings();
}

void SettingsDialog::logSystemEnableCheckBox_slot(int state) {
  if (state == Qt::Checked) {
    LogSystemProxyWidget1->show();
  } else {
    LogSystemProxyWidget1->hide();
  }

  //  adjustSize();
  //  setFixedHeight(size().height());
}

void SettingsDialog::logSystemListenPersoServerCheckBox_slot(int state) {
  if (state == Qt::Checked) {
    LogSystemProxyWidget2->show();
  } else {
    LogSystemProxyWidget2->hide();
  }

  //  adjustSize();
  //  setFixedHeight(size().height());
}

void SettingsDialog::logSystemFileEnableCheckBox_slot(int32_t state) {
  if (state == Qt::Checked) {
    LogSystemProxyWidget3->show();
  } else {
    LogSystemProxyWidget3->hide();
  }

  //  adjustSize();
  //  setFixedHeight(size().height());
}

void SettingsDialog::stickerPrinterLibPathPushButton_slot() {
  QString filePath =
      QFileDialog::getOpenFileName(this, "Выберите файл", "", "*.dll");
  StickerPrinterLibPathLineEdit->setText(filePath);
}
