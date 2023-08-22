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

  /* Меню создания заказов */
  //============================================================
  QWidget* OrderCreationTab;
  QHBoxLayout* OrderCreationTabMainLayout;

  QGroupBox* OrderCreationControlPanel;
  QVBoxLayout* OrderCreationControlPanelLayout;

  QCheckBox* FullPersonalizationCheckBox;

  QWidget* OrderCreationControlPanelSubWidget;
  QHBoxLayout* OrderCreationControlPanelSubLayout;
  QLabel* PanFilePathLabel;
  QLineEdit* PanFilePathLineEdit;
  QPushButton* PanFileExplorePushButton;

  QHBoxLayout* OrderCreationControlPanelSubLayout1;
  QLabel* IssuerNameLabel;
  QComboBox* IssuerNameComboBox;

  QHBoxLayout* OrderCreationControlPanelSubLayout2;
  QLabel* TransponderQuantityLabel;
  QLineEdit* TransponderQuantityLineEdit;

  QPushButton* CreateNewOrderPushButton;
  QSpacerItem* OrderCreationControlPanelVS;

  QGroupBox* PanListPanel;
  QVBoxLayout* PanListLayout;
  QTableView* PanListTableView;

  QSpacerItem* OrderCreationTabMainLayoutHS;
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
  void createServerTab(void);
  void createDataBaseTab(void);
  void createOrderCreationTab(void);
  void createSecurityTab(void);
  void createSettingsTab(void);

 private slots:
  void on_FullPersonalizationCheckBoxChanged(void);

 signals:
};

#endif  // GUI_MASTER_H
