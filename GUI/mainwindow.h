#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

#include "../Control/log_system.h"
#include "../Control/manager.h"
#include "../DTR/md5859.h"
#include "../DTR/tc278.h"
#include "mainwindow_gui.h"
#include "notification_system.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

private:
  MainWindow_GUI *GUI;

  DTR *CurrentDtr;
  DsrcManager *Manager;

  GlobalLogSystem *LogSystem;
  UserInteractionSystem *InteractionSystem;

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void displayLogData(QString *log);

  // Подключение к DTR
  void connectMD5859_slot();
  void connectTC278_slot();

  // Функционал общий для всех DTR
  void connect_slot();
  void disconnect_slot();
  void reboot_slot();
  void status_slot();
  void transmitCustomData_slot();

  // Функционал специфичный для TC278

  // Функционал специфичный для MD5859
  void enableCommunication_slot();
  void disableCommunication_slot();
  void configure_slot();

  // DSRC функционал
  void cardme4Transaction_slot();
  void read_slot();
  void clear_slot();
  void personalize_slot();
  void hardReset_slot();
  void overwriteAttribute_slot();

  // Тестирующий функционал
  void fullTesting_slot();
  void singleTest_slot();
  void appIKTest_slot();
  void appTKTest_slot();
  void efcTest_slot();
  void systemTest_slot();

  // Тестирование аккумуляторной батареи
  void startEndlessMeasuring_slot();
  void stopEndlessMeasuring_slot();
  void measureCountable_slot();

  // Настройки
  void saveSettings_slot();

  // Вспомогательные слоты
  void elementChoice_slot(int index);
  void productionInterfaceRequest_slot(void);
  void masterInterfaceRequest_slot(void);

private:
  void connectInitialInterface(void);
  void connectProductionInterface(void);
  void connectMasterInterface(void);

  void processingDtrConnection(void);
  void setupDsrcManager(void);

signals:
  void notifyUser(const QString &data);
  void notifyUserAboutError(const QString &data);
  void requestUserInputKey(QString &key);
};
#endif // MAINWINDOW_H
