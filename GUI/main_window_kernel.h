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
#include "Management/perso_manager.h"
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

  PersoManager* Manager;
  LogSystem* Logger;
  UserInteractionSystem* InteractionSystem;

  QSettings* Settings;

 public:
  MainWindowKernel(QWidget* parent = nullptr);
  ~MainWindowKernel();

  PersoManager* manager(void);

 public slots:
  void proxyLogging(const QString& log);
  void openMasterInterface_slot(void);

  void startServer_slot(void);
  void stopServer_slot(void);

  // Функционал для работы с базой данных
  void connectDataBase_slot(void);
  void disconnectDataBase_slot(void);
  void transamitCustomRequest_slot(void);

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
  void requestUserInputKey(QString& key);
};
#endif  // MAINWINDOWKERNEL_H
