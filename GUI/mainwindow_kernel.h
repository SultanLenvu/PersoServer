#ifndef MAINWINDOWKERNEL_H
#define MAINWINDOWKERNEL_H

#include <QDebug>
#include <QHash>
#include <QMainWindow>
#include <QRegularExpression>
#include <QSettings>

#include <QString>

#include "Database/sql_query_values.h"
#include "General/hash_model.h"
#include "Log/log_system.h"
#include "Management/admin_manager.h"
#include "abstract_gui.h"
#include "interaction_system.h"

class MainWindowKernel : public QMainWindow {
  Q_OBJECT
 private:
  QSize DesktopGeometry;
  AbstractGUI* CurrentGUI;

  QMenu* ServiceMenu;
  QMenu* HelpMenu;

  QAction* SettingsAction;
  QAction* AboutProgramAction;
  QAction* RequestAuthorizationGuiAction;

  InteractionSystem* Interactor;

  QThread* ManagerThread;
  AdminManager* Manager;

  QThread* LoggerThread;
  LogSystem* Logger;

  SqlQueryValues* RandomModel;
  SqlQueryValues* OrderModel;
  SqlQueryValues* ProductionLineModel;
  SqlQueryValues* IssuerModel;
  SqlQueryValues* TransponderModel;
  SqlQueryValues* StickerModel;

  HashModel* TransponderData;

  QHash<QString, QString>* MatchingTable;

 public:
  MainWindowKernel(QWidget* parent = nullptr);
  ~MainWindowKernel();

 public slots:
  void requestAuthorizationGUIAct_slot(void);

  // Авторизация
  void authorizePushButton_slot(void);

  // Функционал для работы с базой данных
  void connectDatabasePushButton_slot(void);
  void disconnectDatabasePushButton_slot(void);
  void showDatabaseTablePushButton_slot(void);
  void transmitCustomRequestPushButton_slot(void);

  // Функционал для работы с заказами
  void createNewOrderPushButton_slot(void);
  void startOrderAssemblingPushButton_slot(void);
  void stopOrderAssemblingPushButton_slot(void);
  void updateOrderViewPushButton_slot(void);

  // Функционал для работы с производственными линиями
  void createNewProductionLinePushButton_slot(void);
  void startProductionLinePushButton_slot(void);
  void stopProductionLinePushButton_slot(void);
  void deactivateAllProductionLinesPushButton_slot(void);
  void editProductionLinesPushButton_slot(void);
  void updateProductionLineViewPushButton_slot(void);

  // Функционал для работы с транспортными мастер ключами
  void initTransportMasterKeysPushButton_slot(void);
  void initIssuerTablePushButton_slot(void);
  void linkIssuerWithKeysPushButton_slot(void);

  // Функционал для взаимодействия с сервером
  void releaseTransponderPushButton_slot(void);
  void confirmTransponderPushButton_slot(void);
  void rereleaseTransponderPushButton_slot(void);
  void confirmRereleaseTransponderPushButton_slot(void);
  void productionLineRollbackPushButton_slot(void);

  void printBoxStickerOnServerPushButton_slot(void);
  void printLastBoxStickerOnServerPushButton_slot(void);
  void printPalletStickerOnServerPushButton_slot(void);
  void printLastPalletStickerOnServerPushButton_slot(void);

  // Функционал для выпуска, возврата и отгрузки транспондеров
  void transponderManualReleasePushButton_slot(void);
  void transponderManualRefundPushButton_slot(void);
  void palletShipmentPushButton_slot(void);

  // Функционал для работы с принтером стикеров
  void printTransponderStickerPushButton_slot(void);
  void printBoxStickerPushButton_slot(void);
  void printPalletStickerPushButton_slot(void);
  void execStickerPrinterCommandScriptPushButton_slot(void);

  // Функционал для настройки сервера
  void settingsActionTrigger_slot(void);

  // Отображение данных
  void displayFirmware_slot(std::shared_ptr<QFile> firmware);
  void displayTransponderData_slot(
      std::shared_ptr<QHash<QString, QString>> transponderData);

 private:
  Q_DISABLE_COPY_MOVE(MainWindowKernel)
  void loadSettings(void) const;

  void createTopMenu(void);  // Создание верхнего меню
  void createTopMenuActions(void);  // Создание функционала для верхнего меню

  void createAuthorazationGUI(void);
  void connectAuthorizationGUI(void);

  void createMainWindowGUI(void);
  void connectMainWindowGUI(void);

  void createLoggerInstance(void);
  void createManagerInstance(void);
  void createInteractorInstance(void);
  void createModels(void);
  void createMatchingTable(void);

  void registerMetaType(void);

 signals:
  // Логгер
  void loggerClear_signal(void);
  void logging(const QString& log);

  // База данных
  void connectDatabase_signal(void);
  void disconnectDatabase_signal(void);
  void showDatabaseTable_signal(const QString& name, SqlQueryValues* model);
  void performCustomRequest_signal(const QString& req, SqlQueryValues* model);

  // Заказы
  void createNewOrder_signal(
      const std::shared_ptr<QHash<QString, QString>> orderParameterseters,
      SqlQueryValues* model);
  void startOrderAssembling_signal(
      const std::shared_ptr<QHash<QString, QString>>,
      SqlQueryValues* model);
  void stopOrderAssembling_signal(const std::shared_ptr<QHash<QString, QString>>,
                                  SqlQueryValues* model);
  void showOrderTable_signal(SqlQueryValues* model);

  // Производственные линии
  void createNewProductionLine_signal(
      const std::shared_ptr<QHash<QString, QString>>
          productionLineParameterseters,
      SqlQueryValues* model);
  void startProductionLine_signal(const std::shared_ptr<QHash<QString, QString>>,
                                  SqlQueryValues* model);
  void stopProductionLine_signal(const std::shared_ptr<QHash<QString, QString>>,
                                 SqlQueryValues* model);
  void stopAllProductionLines_signal(SqlQueryValues* model);
  void editProductionLine_signal(const std::shared_ptr<QHash<QString, QString>>,
                                 SqlQueryValues* model);
  void showProductionLineTable_signal(SqlQueryValues* model);

  // Тест сервера
  void releaseTransponder_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void confirmTransponderRelease_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void rereleaseTransponder_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void confirmTransponderRerelease_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void rollbackProductionLine_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void printBoxStickerOnServer_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void printLastBoxStickerOnServer_signal();
  void printPalletStickerOnServer_signal(
      const std::shared_ptr<QHash<QString, QString>> param);
  void printLastPalletStickerOnServer_signal();

  // Транспондеры
  void releaseTranspondersManually_signal(
      const std::shared_ptr<QHash<QString, QString>> param,
      SqlQueryValues* model);
  void refundTranspondersManually_signal(
      const std::shared_ptr<QHash<QString, QString>> param,
      SqlQueryValues* model);
  void shipPallets_signal(const std::shared_ptr<QHash<QString, QString>> param,
                          SqlQueryValues* model);

  // Заказчики
  void initIssuers_signal(SqlQueryValues* model);
  void initTransportMasterKeys_signal(SqlQueryValues* model);
  void linkIssuerWithMasterKeys_signal(
      const std::shared_ptr<QHash<QString, QString>> param,
      SqlQueryValues* model);

  // Принтер
  void printTransponderSticker_signal(
      const std::shared_ptr<QHash<QString, QString>> param,
      SqlQueryValues* model);
  void printBoxSticker_signal(
      const std::shared_ptr<QHash<QString, QString>> param,
      SqlQueryValues* model);
  void printPalletSticker_signal(
      const std::shared_ptr<QHash<QString, QString>> param,
      SqlQueryValues* model);
  void execPrinterStickerCommandScript_signal(
      const std::shared_ptr<QStringList> commandScript);
};
#endif  // MAINWINDOWKERNEL_H
