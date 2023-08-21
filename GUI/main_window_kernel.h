#ifndef MAINWINDOWKERNEL_H
#define MAINWINDOWKERNEL_H

#include <QMainWindow>
#include <QSettings>
#include <QString>

#include "gui.h"
#include "gui_initial.h"
#include "gui_master.h"
#include "user_interaction_system.h"

#include "Management/log_system.h"
#include "Management/server_manager.h"
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
  UserInteractionSystem* InteractionSystem;

  QSettings* Settings;

 public:
  MainWindowKernel(QWidget* parent = nullptr);
  ~MainWindowKernel();

  ServerManager* manager(void);

 public slots:
  void proxyLogging(const QString& log);
  void openMasterInterface_slot(void);

  void start_slot(void);
  void stop_slot(void);

  // Функционал для работы с базой данных
  void on_ConnectDataBasePushButton_slot(void);
  void on_DisconnectDataBasePushButton_slot(void);

  void on_ShowProductionLineTablePushButton_slot(void);
  void on_ShowTransponderTablePushButton_slot(void);
  void on_ShowOrderTablePushButton_slot(void);
  void on_ShowIssuerTablePushButton_slot(void);
  void on_ShowBoxTablePushButton_slot(void);
  void on_ShowPalletPushButton_slot(void);

  void on_TransmitCustomRequestPushButton_slot(void);

  // Функционал для настройки сервера
  void applyUserSettings_slot(void);

 private:
  void createTopMenu(void);  // Создание верхнего меню
  void createTopMenuActions(void);  // Создание функционала для верхнего меню

  void createInitialInterface(void);
  void connectInitialInterface(void);

  void createMasterInterface(void);
  void connectMasterInterface(void);

  void setupInterructionSystem(void);
  void setupManager(void);
  void setupLogSystem(void);

 signals:
  void logging(const QString& log);
  void notifyUser(const QString& data);
  void notifyUserAboutError(const QString& data);
  void requestMasterPassword(QString& key);
};
#endif  // MAINWINDOWKERNEL_H
