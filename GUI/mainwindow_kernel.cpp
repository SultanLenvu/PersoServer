#include "mainwindow_kernel.h"
#include "Dialogs/idetifier_input_dialog.h"
#include "Dialogs/link_issuer_key_dialog.h"
#include "Dialogs/manual_release_refund_dialog.h"
#include "Dialogs/order_creation_dialog.h"
#include "Dialogs/pallet_shiping_dialog.h"
#include "Dialogs/pan_input_dialog.h"
#include "Dialogs/production_line_creation_dialog.h"
#include "Dialogs/start_production_line_dialog.h"
#include "GUI/Dialogs/settings_dialog.h"
#include "General/definitions.h"
#include "authorization_gui.h"
#include "mainwindow_gui.h"

MainWindowKernel::MainWindowKernel(QWidget* parent) : QMainWindow(parent) {
  // Считываем размеры дисплея
  DesktopGeometry = QApplication::primaryScreen()->size();

  // Загружаем пользовательские настройки
  loadSettings();

  // Графический интерфейс пока не создан
  CurrentGUI = nullptr;

  // Создаем систему логгирования
  createLoggerInstance();

  // Cистема взаимодействия с пользователем
  createInteractorInstance();

  // Управляющий модуль
  createManagerInstance();

  // Создаем модели для представлений
  createModels();

  // Создаем таблицу соответствий
  createMatchingTable();

  // Создаем графический интерфейс окна авторизации
  createAuthorazationGUI();

  // Регистрируем пользовательские типы в мета-объеткной системе
  registerMetaType();
}

MainWindowKernel::~MainWindowKernel() {
  ManagerThread->quit();
  ManagerThread->wait();

  LoggerThread->quit();
  LoggerThread->wait();
}

void MainWindowKernel::requestAuthorizationGUIAct_slot() {
  createAuthorazationGUI();
}

void MainWindowKernel::authorizePushButton_slot() {
  createMainWindowGUI();

  emit connectDatabase_signal();
}

void MainWindowKernel::connectDatabasePushButton_slot() {
  emit loggerClear_signal();

  emit connectDatabase_signal();
}

void MainWindowKernel::disconnectDatabasePushButton_slot() {
  emit loggerClear_signal();

  emit disconnectDatabase_signal();
}

void MainWindowKernel::showDatabaseTablePushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);
  emit loggerClear_signal();

  emit showDatabaseTable_signal(AbstractGUI->DatabaseTableChoice->currentText(),
                                RandomModel);
}

void MainWindowKernel::transmitCustomRequestPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);
  emit loggerClear_signal();

  emit performCustomRequest_signal(AbstractGUI->CustomRequestLineEdit->text(),
                                   RandomModel);
}

void MainWindowKernel::createNewOrderPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  OrderCreationDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit createNewOrder_signal(param, OrderModel);
}

void MainWindowKernel::startOrderAssemblingPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  IdentifierInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit startOrderAssembling_signal(param, OrderModel);
}

void MainWindowKernel::stopOrderAssemblingPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  IdentifierInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit stopOrderAssembling_signal(param, OrderModel);
}

void MainWindowKernel::updateOrderViewPushButton_slot() {
  emit loggerClear_signal();
  emit showDatabaseTable_signal("orders", OrderModel);
}

void MainWindowKernel::createNewProductionLinePushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  ProductionLineCreationDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit createNewProductionLine_signal(param, ProductionLineModel);
}

void MainWindowKernel::startProductionLinePushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  StartProductionLineDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit startProductionLine_signal(param, ProductionLineModel);
}

void MainWindowKernel::stopProductionLinePushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  IdentifierInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit stopProductionLine_signal(param, ProductionLineModel);
}

void MainWindowKernel::deactivateAllProductionLinesPushButton_slot() {
  emit loggerClear_signal();
  emit stopAllProductionLines_signal(ProductionLineModel);
}

void MainWindowKernel::editProductionLinesPushButton_slot() {
  emit loggerClear_signal();
}

void MainWindowKernel::updateProductionLineViewPushButton_slot() {
  emit loggerClear_signal();
  emit showDatabaseTable_signal("production_lines", ProductionLineModel);
}

void MainWindowKernel::initTransportMasterKeysPushButton_slot() {
  emit loggerClear_signal();

  emit initTransportMasterKeys_signal(IssuerModel);
}

void MainWindowKernel::initIssuerTablePushButton_slot() {
  emit loggerClear_signal();

  emit initIssuers_signal(IssuerModel);
}

void MainWindowKernel::linkIssuerWithKeysPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  LinkIssuerKeyDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit linkIssuerWithMasterKeys_signal(param, IssuerModel);
}

void MainWindowKernel::releaseTransponderPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  emit loggerClear_signal();

  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());
  param->insert("login", AbstractGUI->LoginLineEdit2->text());
  param->insert("password", AbstractGUI->PasswordLineEdit2->text());

  emit releaseTransponder_signal(param);
}

void MainWindowKernel::confirmTransponderPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  emit loggerClear_signal();

  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());
  param->insert("login", AbstractGUI->LoginLineEdit2->text());
  param->insert("password", AbstractGUI->PasswordLineEdit2->text());
  param->insert("ucid", AbstractGUI->UcidLineEdit->text());

  emit confirmTransponderRelease_signal(param);
}

void MainWindowKernel::rereleaseTransponderPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  emit loggerClear_signal();

  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());
  param->insert("login", AbstractGUI->LoginLineEdit2->text());
  param->insert("password", AbstractGUI->PasswordLineEdit2->text());
  param->insert("pan", AbstractGUI->RereleaseKeyLineEdit->text());

  emit rereleaseTransponder_signal(param);
}

void MainWindowKernel::confirmRereleaseTransponderPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  emit loggerClear_signal();

  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());
  param->insert("login", AbstractGUI->LoginLineEdit2->text());
  param->insert("password", AbstractGUI->PasswordLineEdit2->text());
  param->insert("pan", AbstractGUI->RereleaseKeyLineEdit->text());
  param->insert("ucid", AbstractGUI->UcidLineEdit->text());

  emit confirmTransponderRerelease_signal(param);
}

void MainWindowKernel::productionLineRollbackPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  emit loggerClear_signal();

  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());
  param->insert("login", AbstractGUI->LoginLineEdit2->text());
  param->insert("password", AbstractGUI->PasswordLineEdit2->text());

  emit rollbackProductionLine_signal(param);
}

void MainWindowKernel::printBoxStickerOnServerPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());

  PanInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit printBoxStickerOnServer_signal(param);
}

void MainWindowKernel::printLastBoxStickerOnServerPushButton_slot() {
  emit loggerClear_signal();

  emit printLastBoxStickerOnServer_signal();
}

void MainWindowKernel::printPalletStickerOnServerPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());

  PanInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit printPalletStickerOnServer_signal(param);
}

void MainWindowKernel::printLastPalletStickerOnServerPushButton_slot() {
  emit printLastPalletStickerOnServer_signal();
}

void MainWindowKernel::transponderManualReleasePushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());

  ManualReleaseRefundDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit releaseTranspondersManually_signal(param, TransponderModel);
}

void MainWindowKernel::transponderManualRefundPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());

  ManualReleaseRefundDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit refundTranspondersManually_signal(param, TransponderModel);
}

void MainWindowKernel::palletShipmentPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>());

  PalletShippingDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit shipPallets_signal(param, TransponderModel);
}

void MainWindowKernel::printTransponderStickerPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  IdentifierInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit printTransponderSticker_signal(param, StickerModel);
}

void MainWindowKernel::printBoxStickerPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  IdentifierInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit printBoxSticker_signal(param, StickerModel);
}

void MainWindowKernel::printPalletStickerPushButton_slot() {
  std::shared_ptr<QHash<QString, QString>> param(new QHash<QString, QString>);

  IdentifierInputDialog dialog(this);
  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
  dialog.getData(param.get());

  emit loggerClear_signal();
  emit printPalletSticker_signal(param, StickerModel);
}

void MainWindowKernel::execStickerPrinterCommandScriptPushButton_slot() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  emit loggerClear_signal();

  std::shared_ptr<QStringList> commandScript(new QStringList(
      AbstractGUI->StickerPrinterCommandScriptInput->toPlainText().split(
          "\n")));
  emit execPrinterStickerCommandScript_signal(commandScript);
}

void MainWindowKernel::settingsActionTrigger_slot() {
  emit loggerClear_signal();

  SettingsDialog dialog(this);
  connect(&dialog, &SettingsDialog::applyNewSettings, Interactor,
          &InteractionSystem::applySettings);
  connect(&dialog, &SettingsDialog::applyNewSettings, Logger,
          &LogSystem::applySettings);
  connect(&dialog, &SettingsDialog::applyNewSettings, Manager,
          &AdminManager::applySettings);

  if (dialog.exec() == QDialog::Rejected) {
    return;
  }
}

void MainWindowKernel::displayFirmware_slot(std::shared_ptr<QFile> firmware) {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  if (!firmware.get()->open(QIODevice::ReadOnly)) {
    Interactor->generateErrorMessage(
        "Не удалось открыть файл прошивки для отображения.");
  }
  AbstractGUI->AssembledFirmwareView->setPlainText(
      firmware.get()->readAll().toHex());
  firmware.get()->close();
}

void MainWindowKernel::displayTransponderData_slot(
    std::shared_ptr<QHash<QString, QString>> transponderData) {
  TransponderData->buildTransponderData(transponderData.get());
  CurrentGUI->update();
}

/*
 * Приватные методы
 */

void MainWindowKernel::loadSettings() const {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);
}

// bool MainWindowKernel::checkReleaseTransponderInput() const {
//   MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);
//   QRegularExpression regex("[A-Fa-f0-9]+");
//   QString login = AbstractGUI->LoginLineEdit2->text();
//   QString pass = AbstractGUI->PasswordLineEdit2->text();
//   QString ucid = AbstractGUI->UcidLineEdit->text();

//  if (ucid.size() != UCID_CHAR_LENGTH) {
//    return false;
//  }

//  QRegularExpressionMatch match = regex.match(ucid);
//  if ((!match.hasMatch()) || (match.captured(0) != ucid)) {
//    return false;
//  }

//  if ((login.size() == 0) || (login.size() > 20)) {
//    return false;
//  }

//  if ((pass.size() == 0) || (pass.size() > 20)) {
//    return false;
//  }

//  return true;
//}

// bool MainWindowKernel::checkConfirmRereleaseTransponderInput() const {
//   MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);
//   QRegularExpression ucidRegex("[A-Fa-f0-9]+");
//   QRegularExpression panRegex("[0-9]+");
//   QString choice = AbstractGUI->RereleaseKeyComboBox->currentText();
//   QString input = AbstractGUI->RereleaseKeyLineEdit->text();
//   QString ucid = AbstractGUI->UcidLineEdit->text();
//   QString login = AbstractGUI->LoginLineEdit2->text();
//   QString pass = AbstractGUI->PasswordLineEdit2->text();

//  if ((login.size() == 0) || (login.size() > 20)) {
//    return false;
//  }

//  if ((pass.size() == 0) || (pass.size() > 20)) {
//    return false;
//  }

//  if (choice == "SN") {
//    if (input.toInt() == 0) {
//      return false;
//    }
//  } else if (choice == "PAN") {
//    if (input.length() != PAN_CHAR_LENGTH) {
//      return false;
//    }

//    QRegularExpressionMatch match = panRegex.match(input);
//    if ((!match.hasMatch()) || (match.captured(0) != input)) {
//      return false;
//    }
//  } else {
//    return false;
//  }

//  if (ucid.size() != UCID_CHAR_LENGTH) {
//    return false;
//  }

//  QRegularExpressionMatch match = ucidRegex.match(ucid);
//  if ((!match.hasMatch()) || (match.captured(0) != ucid)) {
//    return false;
//  }

//  return true;
//}

// bool MainWindowKernel::checkLinkIssuerInput() const {
//   MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);
//   QString issuerId = AbstractGUI->IssuerIdLineEdit1->text();
//   QString masterKeysId = AbstractGUI->MasterKeysLineEdit1->text();

//  if (issuerId.toInt() == 0) {
//    return false;
//  }

//  if (masterKeysId.toInt() == 0) {
//    return false;
//  }

//  return true;
//}

void MainWindowKernel::createTopMenu() {
  menuBar()->clear();
  createTopMenuActions();

  ServiceMenu = menuBar()->addMenu("Сервис");
  ServiceMenu->addAction(RequestAuthorizationGuiAction);
  ServiceMenu->addAction(SettingsAction);

  HelpMenu = menuBar()->addMenu("Справка");
  HelpMenu->addAction(AboutProgramAction);
}

void MainWindowKernel::createTopMenuActions() {
  SettingsAction = new QAction("Настройки");
  SettingsAction->setStatusTip("Открыть меню настроек");
  connect(SettingsAction, &QAction::triggered, this,
          &MainWindowKernel::settingsActionTrigger_slot);

  RequestAuthorizationGuiAction = new QAction("Авторизация");
  RequestAuthorizationGuiAction->setStatusTip(
      "Закрыть текущий интерфейс и создать начальный интерфейс");
  connect(RequestAuthorizationGuiAction, &QAction::triggered, this,
          &MainWindowKernel::requestAuthorizationGUIAct_slot);

  AboutProgramAction = new QAction("О программе", this);
  AboutProgramAction->setStatusTip("Показать сведения о программе");
}

void MainWindowKernel::createAuthorazationGUI() {
  // Удаляем предыдущий интерфейс
  if (CurrentGUI) {
    CurrentGUI->hide();
    delete CurrentGUI;
  }

  // Создаем виджеты
  CurrentGUI = new AuthorizationGUI(this);
  setCentralWidget(CurrentGUI);
  CurrentGUI->create();

  // Настраиваем размер главного окна
  setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1,
              DesktopGeometry.width() * 0.2, DesktopGeometry.height() * 0.1);

  // Подключаем интерфейс
  connectAuthorizationGUI();
}

void MainWindowKernel::connectAuthorizationGUI() {
  AuthorizationGUI* AbstractGUI = dynamic_cast<AuthorizationGUI*>(CurrentGUI);

  connect(AbstractGUI->AuthorizePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::authorizePushButton_slot);
}

void MainWindowKernel::createMainWindowGUI() {
  // Удаляем предыдущий интерфейс
  if (CurrentGUI) {
    CurrentGUI->hide();
    delete CurrentGUI;
  }
  // Настраиваем размер главного окна
  setGeometry(DesktopGeometry.width() * 0.1, DesktopGeometry.height() * 0.1,
              DesktopGeometry.width() * 0.5, DesktopGeometry.height() * 0.5);

  // Создаем интерфейс
  CurrentGUI = new MainWindowGUI(this);
  setCentralWidget(CurrentGUI);
  CurrentGUI->create();

  // Создаем верхнее меню
  createTopMenu();

  // Подключаем интерфейс
  connectMainWindowGUI();
}

void MainWindowKernel::connectMainWindowGUI() {
  MainWindowGUI* AbstractGUI = dynamic_cast<MainWindowGUI*>(CurrentGUI);

  // База данных
  connect(AbstractGUI->ConnectDatabasePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::connectDatabasePushButton_slot);
  connect(AbstractGUI->DisconnectDatabasePushButton, &QPushButton::clicked,
          this, &MainWindowKernel::disconnectDatabasePushButton_slot);
  connect(AbstractGUI->ShowDatabaseTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::showDatabaseTablePushButton_slot);
  connect(AbstractGUI->InitIssuerTablePushButton, &QPushButton::clicked, this,
          &MainWindowKernel::initIssuerTablePushButton_slot);

  connect(AbstractGUI->TransmitCustomRequestPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::transmitCustomRequestPushButton_slot);

  // Заказы
  connect(AbstractGUI->CreateNewOrderPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::createNewOrderPushButton_slot);
  connect(AbstractGUI->StartOrderAssemblingPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::startOrderAssemblingPushButton_slot);
  connect(AbstractGUI->StopOrderAssemblingPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::stopOrderAssemblingPushButton_slot);
  connect(AbstractGUI->UpdateOrderViewPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::updateOrderViewPushButton_slot);

  // Производственные линии
  connect(AbstractGUI->CreateNewProductionLinePushButton, &QPushButton::clicked,
          this, &MainWindowKernel::createNewProductionLinePushButton_slot);
  connect(AbstractGUI->StartProductionLinePushButton, &QPushButton::clicked,
          this, &MainWindowKernel::startProductionLinePushButton_slot);
  connect(AbstractGUI->StopProductionLinePushButton, &QPushButton::clicked,
          this, &MainWindowKernel::stopProductionLinePushButton_slot);
  connect(AbstractGUI->ShutdownAllProductionLinesPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::deactivateAllProductionLinesPushButton_slot);
  connect(AbstractGUI->UpdateProductionLineViewPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::updateProductionLineViewPushButton_slot);

  // Эмитенты
  connect(AbstractGUI->InitTransportMasterKeysPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::initTransportMasterKeysPushButton_slot);
  connect(AbstractGUI->LinkIssuerWithKeysPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::linkIssuerWithKeysPushButton_slot);

  // Тест сервера
  connect(AbstractGUI->ReleaseTransponderPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::releaseTransponderPushButton_slot);
  connect(AbstractGUI->ConfirmTransponderPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::confirmTransponderPushButton_slot);
  connect(AbstractGUI->RereleaseTransponderPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::rereleaseTransponderPushButton_slot);
  connect(AbstractGUI->ConfirmRereleaseTransponderPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::confirmRereleaseTransponderPushButton_slot);
  connect(AbstractGUI->ProductionLineRollbackPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::productionLineRollbackPushButton_slot);

  connect(AbstractGUI->PrintBoxStickerOnServerPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::printBoxStickerOnServerPushButton_slot);
  connect(AbstractGUI->PrintLastBoxStickerOnServerPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::printLastBoxStickerOnServerPushButton_slot);
  connect(AbstractGUI->PrintPalletStickerOnServerPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::printPalletStickerOnServerPushButton_slot);
  connect(AbstractGUI->PrintLastPalletStickerOnServerPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::printLastPalletStickerOnServerPushButton_slot);

  // Транспондеры
  connect(AbstractGUI->TransponderManualReleasePushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::transponderManualReleasePushButton_slot);
  connect(AbstractGUI->TransponderManualRefundPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::transponderManualRefundPushButton_slot);
  connect(AbstractGUI->PalletShipmentPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::palletShipmentPushButton_slot);

  // Стикеры
  connect(AbstractGUI->PrintTransponderStickerPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::printTransponderStickerPushButton_slot);
  connect(AbstractGUI->PrintBoxStickerPushButton, &QPushButton::clicked, this,
          &MainWindowKernel::printBoxStickerPushButton_slot);
  connect(AbstractGUI->PrintPalletStickerPushButton, &QPushButton::clicked,
          this, &MainWindowKernel::printPalletStickerPushButton_slot);
  connect(AbstractGUI->ExecStickerPrinterCommandScriptPushButton,
          &QPushButton::clicked, this,
          &MainWindowKernel::execStickerPrinterCommandScriptPushButton_slot);

  // Подключаем логгер
  connect(Logger->getWidgetLogger(), &WidgetLogBackend::displayLog_signal,
          AbstractGUI, &MainWindowGUI::displayLog);
  connect(Logger->getWidgetLogger(), &WidgetLogBackend::clearLogDisplay_signal,
          AbstractGUI, &MainWindowGUI::clearLogDisplay);

  // Соединяем модели и представления
  AbstractGUI->DatabaseRandomModelView->setModel(RandomModel);
  AbstractGUI->OrderTableView->setModel(OrderModel);
  AbstractGUI->ProductionLineTableView->setModel(ProductionLineModel);
  AbstractGUI->IssuerTableView->setModel(IssuerModel);
  AbstractGUI->StickerDataTableView->setModel(StickerModel);
  AbstractGUI->TransponderTableView->setModel(TransponderModel);

  AbstractGUI->TransponderDataTableView->setModel(TransponderData);

  // Связываем отображения графиков с логикой их формирования
}

void MainWindowKernel::createLoggerInstance() {
  Logger = LogSystem::instance();
  connect(this, &MainWindowKernel::loggerClear_signal, Logger,
          &LogSystem::clear);
  connect(this, &MainWindowKernel::logging, Logger, &LogSystem::generate);

  LoggerThread = new QThread(this);
  connect(LoggerThread, &QThread::finished, LoggerThread,
          &QThread::deleteLater);

  Logger->moveToThread(LoggerThread);
  LoggerThread->start();
}

void MainWindowKernel::createManagerInstance() {
  Manager = new AdminManager(nullptr);
  connect(Manager, &AdminManager::logging, Logger, &LogSystem::generate);
  connect(Manager, &AdminManager::notifyUser, Interactor,
          &InteractionSystem::generateMessage);
  connect(Manager, &AdminManager::notifyUserAboutError, Interactor,
          &InteractionSystem::generateErrorMessage, Qt::QueuedConnection);
  connect(Manager, &AdminManager::operationPerfomingStarted, Interactor,
          &InteractionSystem::startOperationProgressDialog);
  connect(Manager, &AdminManager::operationPerformingFinished, Interactor,
          &InteractionSystem::finishOperationProgressDialog);

  // Подключаем функционал
  connect(this, &MainWindowKernel::connectDatabase_signal, Manager,
          &AdminManager::connectDatabase);
  connect(this, &MainWindowKernel::disconnectDatabase_signal, Manager,
          &AdminManager::disconnectDatabase);
  connect(this, &MainWindowKernel::showDatabaseTable_signal, Manager,
          &AdminManager::showDatabaseTable);
  connect(this, &MainWindowKernel::performCustomRequest_signal, Manager,
          &AdminManager::performCustomRequest);

  connect(this, &MainWindowKernel::createNewOrder_signal, Manager,
          &AdminManager::createNewOrder);
  connect(this, &MainWindowKernel::startOrderAssembling_signal, Manager,
          &AdminManager::startOrderAssembling);
  connect(this, &MainWindowKernel::stopOrderAssembling_signal, Manager,
          &AdminManager::stopOrderAssembling);
  connect(this, &MainWindowKernel::showOrderTable_signal, Manager,
          &AdminManager::showOrderTable);

  connect(this, &MainWindowKernel::createNewProductionLine_signal, Manager,
          &AdminManager::createNewProductionLine);
  connect(this, &MainWindowKernel::startProductionLine_signal, Manager,
          &AdminManager::startProductionLine);
  connect(this, &MainWindowKernel::stopProductionLine_signal, Manager,
          &AdminManager::stopProductionLine);
  connect(this, &MainWindowKernel::showProductionLineTable_signal, Manager,
          &AdminManager::showProductionLineTable);
  connect(this, &MainWindowKernel::stopAllProductionLines_signal, Manager,
          &AdminManager::stopAllProductionLines);
  connect(this, &MainWindowKernel::showProductionLineTable_signal, Manager,
          &AdminManager::showProductionLineTable);

  connect(this, &MainWindowKernel::releaseTransponder_signal, Manager,
          &AdminManager::releaseTransponder);
  connect(this, &MainWindowKernel::confirmTransponderRelease_signal, Manager,
          &AdminManager::confirmTransponderRelease);
  connect(this, &MainWindowKernel::rereleaseTransponder_signal, Manager,
          &AdminManager::rereleaseTransponder);
  connect(this, &MainWindowKernel::confirmTransponderRerelease_signal, Manager,
          &AdminManager::confirmTransponderRerelease);
  connect(this, &MainWindowKernel::rollbackProductionLine_signal, Manager,
          &AdminManager::rollbackProductionLine);
  connect(this, &MainWindowKernel::printBoxStickerOnServer_signal, Manager,
          &AdminManager::printBoxStickerOnServer);
  connect(this, &MainWindowKernel::printLastBoxStickerOnServer_signal, Manager,
          &AdminManager::printLastBoxStickerOnServer);
  connect(this, &MainWindowKernel::printPalletStickerOnServer_signal, Manager,
          &AdminManager::printPalletStickerOnServer);
  connect(this, &MainWindowKernel::printLastPalletStickerOnServer_signal,
          Manager, &AdminManager::printLastPalletStickerOnServer);

  connect(this, &MainWindowKernel::releaseTranspondersManually_signal, Manager,
          &AdminManager::releaseTranspondersManually);
  connect(this, &MainWindowKernel::refundTranspondersManually_signal, Manager,
          &AdminManager::refundTranspondersManually);
  connect(this, &MainWindowKernel::shipPallets_signal, Manager,
          &AdminManager::shipPallets);

  connect(this, &MainWindowKernel::initIssuers_signal, Manager,
          &AdminManager::initIssuers);
  connect(this, &MainWindowKernel::initTransportMasterKeys_signal, Manager,
          &AdminManager::initTransportMasterKeys);
  connect(this, &MainWindowKernel::linkIssuerWithMasterKeys_signal, Manager,
          &AdminManager::linkIssuerWithMasterKeys);

  connect(this, &MainWindowKernel::printTransponderSticker_signal, Manager,
          &AdminManager::printTransponderSticker);
  connect(this, &MainWindowKernel::printBoxSticker_signal, Manager,
          &AdminManager::printBoxSticker);
  connect(this, &MainWindowKernel::printPalletSticker_signal, Manager,
          &AdminManager::printPalletSticker);
  connect(this, &MainWindowKernel::execPrinterStickerCommandScript_signal,
          Manager, &AdminManager::execPrinterStickerCommandScript);

  // Запросы на отображение
  connect(Manager, &AdminManager::displayFirmware_signal, this,
          &MainWindowKernel::displayFirmware_slot);
  connect(Manager, &AdminManager::displayTransponderData_signal, this,
          &MainWindowKernel::displayTransponderData_slot);

  // Поток
  ManagerThread = new QThread(this);
  connect(ManagerThread, &QThread::finished, ManagerThread,
          &QThread::deleteLater);
  connect(ManagerThread, &QThread::finished, Manager,
          &AdminManager::deleteLater);
  connect(ManagerThread, &QThread::started, Manager,
          &AdminManager::insctanceThreadStarted_slot);

  Manager->moveToThread(ManagerThread);
  ManagerThread->start();
}

void MainWindowKernel::createInteractorInstance() {
  Interactor = InteractionSystem::instance();
  connect(Interactor, &InteractionSystem::logging, Logger,
          &LogSystem::generate);
}

void MainWindowKernel::createModels() {
  RandomModel = new SqlQueryValues(this);
  OrderModel = new SqlQueryValues(this);
  ProductionLineModel = new SqlQueryValues(this);
  IssuerModel = new SqlQueryValues(this);
  StickerModel = new SqlQueryValues(this);
  TransponderModel = new SqlQueryValues(this);

  TransponderData = new HashModel(this);
}

void MainWindowKernel::createMatchingTable() {
  MatchingTable = new QHash<QString, QString>;
  MatchingTable->insert("Транспортные мастер ключи", "transport_master_keys");
  MatchingTable->insert("Коммерческие мастер ключи", "commercial_master_keys");
  MatchingTable->insert("Эмитенты", "issuers");
}

void MainWindowKernel::registerMetaType() {
  qRegisterMetaType<std::shared_ptr<QMap<QString, QString>>>(
      "std::shared_ptr<QHash<QString, QString> >");
  qRegisterMetaType<std::shared_ptr<QHash<QString, QString>>>(
      "std::shared_ptr<QHash<QString, QString> >");
  qRegisterMetaType<std::shared_ptr<QStringList>>(
      "std::shared_ptr<QStringList>");
  qRegisterMetaType<std::shared_ptr<QFile>>("std::shared_ptr<QFile>");
}
