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
#include "Management/transponder_info_model.h"
#include "Management/user_settings.h"

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
  QAction* MasterInterfaceRequestAct;

  ServerManager* Manager;
  LogSystem* Logger;
  UserInteractionSystem* Interactor;

  DatabaseTableModel* RandomModel;
  DatabaseTableModel* OrderModel;
  DatabaseTableModel* ProductionLineModel;
  TransponderInfoModel* TransponderSeed;

 public:
  MainWindowKernel(QWidget* parent = nullptr);
  ~MainWindowKernel();

  ServerManager* manager(void);

 public slots:
  void openMasterInterface_slot(void);

  void start_slot(void);
  void stop_slot(void);

  // Функционал для работы с базой данных
  void on_ConnectDatabasePushButton_slot(void);
  void on_DisconnectDatabasePushButton_slot(void);

  void on_ShowDatabaseTablePushButton_slot(void);
  void on_ClearDatabaseTablePushButton_slot(void);
  void on_InitIssuerTablePushButton_slot(void);
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

  // Функционал для настройки сервера
  void on_ApplySettingsPushButton_slot(void);

 private:
  void loadSettings(void) const;
  bool checkNewSettings(void) const;
  bool checkNewOrderInput(void) const;
  bool checkNewProductionLineInput(void) const;
  bool checkReleaseTransponderInput(void) const;
  bool checkSearchTransponderInput() const;
  bool checkRereleaseTransponderInput() const;

  void createTopMenu(void);  // Создание верхнего меню
  void createTopMenuActions(void);  // Создание функционала для верхнего меню

  void createInitialInterface(void);
  void connectInitialInterface(void);

  void createMasterInterface(void);
  void connectMasterInterface(void);

  void setupInterructionSystem(void);
  void setupManager(void);
  void setupLogSystem(void);
  void createModels(void);

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
};
#endif  // MAINWINDOWKERNEL_H
