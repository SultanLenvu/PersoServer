#ifndef MAINWINDOW_GUI_H
#define MAINWINDOW_GUI_H

#include <QtCharts>
#include <QtCore/QVariant>
#include <QtWidgets>

#include "../Control/manager.h"
#include "delegates_gui.h"

class MainWindow_GUI : public QObject {
public:
  enum DtrType { TC278, MD5859 };

  enum OperatingMode { Initial, Master, Production };

public:
  /* Главное окно приложения */
  //=========================================================================================
  QMainWindow *MainWindow;
  QRect DesktopGeometry;

  // Поключенный в данный момент DTR
  DtrType CurrentDtr;
  // Текущий режим
  OperatingMode CurrentMode;

  /* Виджеты верхней панели меню */
  //==================================================
  QMenu *ServiceMenu;
  QMenu *HelpMenu;

  QAction *MasterInterfaceRequestAct;
  QAction *ProductionInterfaceRequestAct;
  QAction *AboutProgramAct;
  //==================================================
  //=========================================================================================

  /* Корневые виджет всех реализуемых графических интерфейсов */
  //=========================================================================================
  QWidget *CurrentInterface;
  //=========================================================================================

  /* Виджеты стартового интерфейса */
  //=========================================================================================
  QHBoxLayout *II_MainLayout;

  QGroupBox *II_ConnectButtonGroup;
  QVBoxLayout *II_ConnectButtonLayout;
  QPushButton *PushButtonConnectMD5859;
  QPushButton *PushButtonConnectTC278;
  QSpacerItem *II_ConnectButtonSpacer;
  //=========================================================================================

  /* Виджеты производственного интерфейса */
  //=========================================================================================
  QHBoxLayout *PI_MainLayout;

  QGroupBox *PI_ButtonGroup;
  QVBoxLayout *PI_ButtonLayout;
  QPushButton *PI_PushButtonPersonalize;
  QSpacerItem *PI_ButtonSpacer;
  QPushButton *PI_PushButtonHardReset;

  QGroupBox *PI_ObuData;
  QHBoxLayout *PI_ObuDataLayout;
  QTableView *PI_MI_SystemElementView;
  QTableView *PI_EfcElementView;
  //=========================================================================================

  /* Виджеты мастер интерфейса */
  //=========================================================================================
  QHBoxLayout *MI_MainLayout;
  QTabWidget *MI_Tabs;

  /* Виджеты интерфейса настройки DTR */
  //============================================================
  QGroupBox *GroupLog;
  QHBoxLayout *LogLayout;
  QPlainTextEdit *GeneralLogs;
  //============================================================

  /* Виджеты интерфейса настройки DTR */
  //============================================================
  QWidget *DtrTab;
  QHBoxLayout *DtrMainLayout;
  QGroupBox *DtrControlPanelGroupBox;
  QVBoxLayout *DtrControlPanelLayout;

  // Виджеты, общие для всех DTR
  QPushButton *PushButtonConnect;
  QPushButton *PushButtonDisconnect;
  QPushButton *PushButtonReboot;
  QPushButton *PushButtonStatus;
  QPushButton *PushButtonTransmitCustomData;
  QLineEdit *LineEditCustomData;
  QSpacerItem *DtrButtonLayoutVS;

  // Представление для отображения параметров DTR
  QTableView *DtrParametersView;

  // Виджеты, специфичные для TC278
  QPushButton *PushButtonEnableSynchronization;
  QPushButton *PushButtonDisableSynchronization;
  QPushButton *PushButtonSetAttenuation;
  QPushButton *PushButtonGetAttenuation;

  // Виджеты, специфичные для MD5859
  QPushButton *PushButtonEnableCommunication;
  QPushButton *PushButtonDisableCommunication;
  QPushButton *PushButtonConfigure;
  //============================================================

  /* Виджеты интерфейса персонализации */
  //============================================================
  QWidget *PersonalizationTab;
  QHBoxLayout *PersonalizationMainLayout;
  QVBoxLayout *PersonalizationMainSubLayout;

  QGroupBox *PersonalizationControlPanelGroupBox;
  QHBoxLayout *PersonalizationButtonMainLayout;
  QVBoxLayout *PersonalizationButtonSubLayout1;
  QVBoxLayout *PersonalizationButtonSubLayout2;
  QVBoxLayout *PersonalizationButtonSubLayout3;

  QGroupBox *MI_ObuDataGroupBox;
  QHBoxLayout *MI_ObuDataViewLayout;
  QTableView *MI_SystemElementView;
  QTableView *MI_EfcElementView;

  QPushButton *PushButtonRead;
  QPushButton *MI_PushButtonPersonalize;
  QSpacerItem *PersonalizationVerticalSpacer1;

  QPushButton *PushButtonClear;
  QPushButton *MI_PushButtonHardReset;
  QSpacerItem *PersonalizationVerticalSpacer2;

  QComboBox *ElementChoice;
  QComboBox *AttributeChoice;
  QLineEdit *AttributeNewValue;
  QPushButton *PushButtonOverwriteAttribute;
  QSpacerItem *PersonalizationVerticalSpacer3;
  //============================================================

  /* Виджеты интерфейса тестирования */
  //============================================================
  QWidget *TestingTab;
  QHBoxLayout *TestingMainLayout;

  QGroupBox *TestingControlPanelGroupBox;
  QVBoxLayout *TestingControlPanelLayout;
  QPushButton *PushButtonFullTesting;
  QPushButton *PushButtonAppIKTest;
  QPushButton *PushButtonAppTKTest;
  QPushButton *PushButtonEfcTest;
  QPushButton *PushButtonSystemTest;
  QPushButton *PushButtonSingleTest;
  QLineEdit *LineEditTestName;
  QSpacerItem *VerticalSpacer1;
  QPushButton *PushButtonDsrcTransaction;

  QGroupBox *TestResultsGroupBox;
  QGridLayout *TestGridLayout;
  QLabel *AppIKTestsLabel;
  QLabel *AppTKTestsLabel;
  QLabel *EfcTestsLabel;
  QLabel *SystemTestsLabel;
  QTableView *AppIKTestsView;
  QTableView *AppTKTestsView;
  QTableView *EfcTestsView;
  QTableView *SystemTestsView;
  //============================================================

  /* Виджеты интерфейса напряжения питания */
  //============================================================
  QWidget *SupplyVoltageTab;
  QHBoxLayout *SVMainLayout;

  QGroupBox *SVControlPanelGroupBox;
  QVBoxLayout *SVControlPanelLayout;
  QPushButton *PushButtonStartEndlessMeasuring;
  QPushButton *PushButtonStopEndlessMeasuring;
  QPushButton *PushButtonMeasureCountableTimes;
  QLineEdit *LineEditMeasureCount;
  QSpacerItem *VerticalSpacer2;

  QChartView *BatteryVoltageChartView;
  //============================================================

  /* Виджеты интерфейса данных безопасности */
  //============================================================
  QWidget *SecurityTab;
  QGridLayout *SecurityLayout;

  QLabel *TMasterKeysLabel;
  QLabel *CMasterKeysLabel;
  QLabel *CommonKeysLabel;
  QTableView *TMasterKeysView;
  QTableView *CMasterKeysView;
  QTableView *CommonKeysView;
  //============================================================

  /* Виджеты общей настройки программы */
  //============================================================
  QWidget *SettingsTab;
  QHBoxLayout *SettingsMainLayout;
  QVBoxLayout *SettingsMainSubLayout;
  QPushButton *PushButtonSaveSettings;

  QSpacerItem *SettingsHorizontalSpacer1;
  QSpacerItem *SettingsVerticalSpacer1;

  // Настройки для персонализации
  QGroupBox *PersoSettingsGroupBox;
  QGridLayout *PersoSettingsMainLayout;
  QLabel *UseServerPersoLabel;
  QCheckBox *UseServerPersoCheckBox;
  QLabel *ServerCommonKeyGenerationLabel;
  QCheckBox *ServerCommonKeyGenerationCheckBox;
  QLabel *ipAddressPersoServerLabel;
  QLineEdit *ipAddressPersoServerLineEdit;
  QLabel *localMasterKeyPathLabel;
  QLineEdit *localMasterKeyPathLineEdit;
  //============================================================
  //=========================================================================================

public:
  explicit MainWindow_GUI(QObject *parent, QMainWindow *mainWindow);
  ~MainWindow_GUI();

  void setCurrentDtr(DtrType dtr);

  void createInterface(OperatingMode mode); // Создание основного интерфейса
  void destroyInterface(void); // Уничтожение основного интерфейса

  void updateTestTableViews(void); // Обновление представлений
  void updateObuDataViews(void);
  void updateObuKeysViews(void);

private:
  void createInitialInterface(void); // Создание инициирующего интерфейса
  void createProductionInterface(void); // Создание производственного интерфейса
  void createMasterInterface(void); // Создание мастер интерфейса

  void createTopMenu(void); // Создание верхнего меню
  void createTopMenuActions(void); // Создание функционала для верхнего меню

  void create_II_Buttons(void); // Создание кнопкок для подключения к DTR

  void create_PI_Buttons(void); // Создание кнопкок для производства
  void
  create_PI_ObuDataView(void); // Создание отображений для данных транспондераы

  void create_MI_Tabs(void); // Настройка вкладок функциональности

  void createDtrTab(void);
  void
  createTC278SpecificWidgets(void); // Создание виджетов, специфичных для TC278
  void createMD5859SpecificWidgets(
      void); // Создание виджетов, специфичных для MD5859
  void createPersonalizationTab(void); // Создание интерфейса персонализации
  void createTestingTab(void); // Создание интерфейса тестирования
  void createPowerConsumptionTab(
      void); // Создание интерфейса для замеров потребления
  void createSecurityTab(void); // Создание интерфейса данных безопасности
  void createSettingsTab(void); // Создание интерфейса для настройки программы

  void create_MI_LogWidgets(void); // Создание виджетов для отображения логов
};

#endif // MAINWINDOW_GUI_H
