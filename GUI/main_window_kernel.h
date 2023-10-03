#ifndef MAINWINDOWKERNEL_H
#define MAINWINDOWKERNEL_H

#include <QMainWindow>
#include <QMap>
#include <QRegularExpression>
#include <QSettings>
#include <QString>

#include "gui.h"
#include "gui_initial.h"
#include "gui_master.h"
#include "user_interaction_system.h"

#include "Database/database_table_model.h"
#include "GUI/log_system.h"
#include "Management/server_manager.h"
#include "Management/transponder_seed_model.h"

#include "General/definitions.h"

class MainWindowKernel : public QMainWindow {
  Q_OBJECT
 private:
  QRect DesktopGeometry;
  GUI* CurrentGUI;

  QMenu* ServiceMenu;
  QMenu* HelpMenu;

  QAction* ProductionInterfaceRequestAct;
  QAction* AboutProgramAct;
  QAction* OpenInitialGuiRequestAct;

  ServerManager* Manager;
  LogSystem* Logger;
  UserInteractionSystem* Interactor;

  DatabaseTableModel* RandomModel;
  DatabaseTableModel* OrderModel;
  DatabaseTableModel* ProductionLineModel;
  DatabaseTableModel* IssuerModel;

  TransponderSeedModel* TransponderSeed;

  QMap<QString, QString>* MatchingTable;

 public:
  MainWindowKernel(QWidget* parent = nullptr);
  ~MainWindowKernel();

  ServerManager* manager(void);

 public slots:
  void on_OpenMasterGuiPushButton_slot(void);
  void on_OpenInitialGuiRequestAct_slot(void);

  void on_ServerStartPushButton_slot(void);
  void on_ServerStopPushButton_slot(void);

  // Функционал для работы с базой данных
  void on_ConnectDatabasePushButton_slot(void);
  void on_DisconnectDatabasePushButton_slot(void);
  void on_ShowDatabaseTablePushButton_slot(void);
  void on_ClearDatabaseTablePushButton_slot(void);
  void on_TransmitCustomRequestPushButton_slot(void);

  // Функционал для работы с заказами
  void on_CreateNewOrderPushButton_slot(void);
  void on_StartOrderAssemblingPushButton_slot(void);
  void on_StopOrderAssemblingPushButton_slot(void);
  void on_UpdateOrderViewPushButton_slot(void);
  void on_DeleteLastOrderPushButton_slot(void);

  // Функционал для работы с производственными линиями
  void on_CreateNewProductionLinePushButton_slot(void);
  void on_AllocateInactiveProductionLinesPushButton_slot(void);
  void on_LinkProductionLinePushButton_slot(void);
  void on_DeactivateAllProductionLinesPushButton_slot(void);
  void on_UpdateProductionLineViewPushButton_slot(void);
  void on_DeleteLastProductionLinePushButton_slot(void);

  // Функционал для работы с транспондерами
  void on_ReleaseTransponderPushButton_slot(void);
  void on_ConfirmTransponderPushButton_slot(void);
  void on_RereleaseTransponderPushButton_slot(void);
  void on_ConfirmRereleaseTransponderPushButton_slot(void);
  void on_SearchTransponderPushButton_slot(void);
  void on_RefundTransponderPushButton_slot(void);

  // Функционал для работы с транспортными мастер ключами
  void on_ShowIssuerTablePushButton_slot(void);
  void on_InitTransportMasterKeysPushButton_slot(void);
  void on_InitIssuerTablePushButton_slot(void);
  void on_LinkIssuerWithKeysPushButton_slot(void);

  // Функционал для настройки сервера
  void on_ApplySettingsPushButton_slot(void);

 private:
  void loadSettings(void) const;
  bool checkNewSettings(void) const;
  bool checkNewOrderInput(void) const;
  bool checkNewProductionLineInput(void) const;
  bool checkReleaseTransponderInput(void) const;
  bool checkSearchTransponderInput(void) const;
  bool checkConfirmRereleaseTransponderInput(void) const;
  bool checkLinkIssuerInput(void) const;

  void createTopMenu(void);  // Создание верхнего меню
  void createTopMenuActions(void);  // Создание функционала для верхнего меню

  void createInitialInterface(void);
  void connectInitialInterface(void);

  void createMasterInterface(void);
  void connectMasterInterface(void);

  void setupLogger(void);
  void setupInterructionSystem(void);
  void setupManager(void);
  void createModels(void);
  void createMatchingTable(void);

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
};
#endif  // MAINWINDOWKERNEL_H
