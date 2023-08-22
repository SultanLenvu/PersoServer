#ifndef GUI_MASTER_H
#define GUI_MASTER_H

#include <QObject>
#include <QtCharts>

#include "gui.h"
#include "gui_delegates.h"

class MasterGUI : public GUI {
  Q_OBJECT

 public:
  QTabWidget* Tabs;

  /* Интерфейс базы данных */
  //============================================================
  QWidget* DataBaseTab;
  QHBoxLayout* DataBaseMainLayout;

  // Панель упралвения БД
  QGroupBox* DataBaseControlPanelGroup;
  QVBoxLayout* DataBaseControlPanelLayout;

  QPushButton* ConnectDataBasePushButton;
  QPushButton* DisconnectDataBasePushButton;
  QSpacerItem* PushButtonLayoutVS1;

  QPushButton* ShowProductionLineTablePushButton;
  QPushButton* ShowTransponderTablePushButton;
  QPushButton* ShowOrderTablePushButton;
  QPushButton* ShowIssuerTablePushButton;
  QPushButton* ShowBoxTablePushButton;
  QPushButton* ShowPalletPushButton;
  QSpacerItem* PushButtonLayoutVS2;

  QPushButton* TransmitCustomRequestPushButton;
  QLineEdit* CustomRequestLineEdit;

  // Отображение записей в БД
  QGroupBox* DataBaseBufferGroup;
  QVBoxLayout* DataBaseBufferLayout;
  QTableView* DataBaseBufferView;
  //============================================================

  /* Меню инициализации транспондеров */
  //============================================================
  QWidget* ObuInitTab;
  QHBoxLayout* ObuInitTabMainLayout;

  QGroupBox* ObuInitControlPanel;
  QVBoxLayout* ObuInitControlPanelLayout;

  QLabel* InitFilePathLabel;
  QButtonGroup* InitFileFormatChoiceButtonGroup;
  QRadioButton* PanFormatRadioButton;
  QRadioButton* SnPanFormatRadioButton;

  QHBoxLayout* ObuInitControlPanelSubLayout;
  QLineEdit* InitFilePathLineEdit;
  QPushButton* InitFileExplorePushButton;

  QPushButton* InitNewObuListPushButton;
  QSpacerItem* ObuInitControlPanelVS;

  QGroupBox* NewObuListPanel;
  QVBoxLayout* NewObuListLayout;
  QTableView* NewObuListTableView;

  QSpacerItem* ObuInitTabMainLayoutHS;
  //============================================================

  /* Настройки безопасности */
  //============================================================
  QWidget* SecurityTab;
  QGridLayout* SecurityTabMainLayout;

  QLabel* TMasterKeysLabel;
  QLabel* CMasterKeysLabel;
  QLabel* CommonKeysLabel;
  QTableView* TMasterKeysView;
  QTableView* CMasterKeysView;
  QTableView* CommonKeysView;
  //============================================================

  /* Настройки сервера */
  //============================================================
  QWidget* SettingsTab;
  QHBoxLayout* SettingsMainLayout;
  QVBoxLayout* SettingsMainSubLayout;
  QPushButton* ApplySettingsPushButton;

  QSpacerItem* SettingsHorizontalSpacer1;
  QSpacerItem* SettingsVerticalSpacer1;

  // Настройки сервера
  QGroupBox* PersoServerSettingsGroupBox;
  QGridLayout* PersoServerSettingsLayout;
  QLabel* PersoServerIpLabel;
  QLineEdit* PersoServerIpLineEdit;
  QLabel* PersoServerPortLabel;
  QLineEdit* PersoServerPortLineEdit;

  // Настройки базы данных
  QGroupBox* DatabaseSettingsGroupBox;
  QGridLayout* DatabaseSettingsLayout;
  QLabel* DatabaseIpLabel;
  QLineEdit* DatabaseIpLineEdit;
  QLabel* DatabasePortLabel;
  QLineEdit* DatabasePortLineEdit;
  QLabel* DatabaseNameLabel;
  QLineEdit* DatabaseNameLineEdit;
  QLabel* DatabaseUserNameLabel;
  QLineEdit* DatabaseUserNameLineEdit;
  QLabel* DatabaseUserPasswordLabel;
  QLineEdit* DatabaseUserPasswordLineEdit;
  //============================================================
 public:
  explicit MasterGUI(QObject* parent);

  virtual QWidget* create(void) override;
  virtual void update(void) override;

 private:
  void createTabs(void);
  void createDataBaseTab(void);
  void createObuInitializationTab(void);
  void createSecurityTab(void);
  void createSettingsTab(void);

 signals:
};

#endif  // GUI_MASTER_H
