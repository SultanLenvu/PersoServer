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

  /* Общий интерфейс базы данных */
  //============================================================
  QWidget* DatabaseTab;
  QHBoxLayout* DatabaseMainLayout;

  // Панель упралвения БД
  QGroupBox* DatabaseControlPanelGroup;
  QVBoxLayout* DatabaseControlPanelLayout;

  QPushButton* ConnectDatabasePushButton;
  QPushButton* DisconnectDatabasePushButton;
  QSpacerItem* DatabaseControlPanelVS;

  QComboBox* DatabaseTableChoice;
  QPushButton* ShowDatabaseTablePushButton;
  QPushButton* ClearDatabaseTablePushButton;
  QPushButton* InitIssuerTablePushButton;
  QSpacerItem* DatabaseControlPanelVS1;

  QPushButton* TransmitCustomRequestPushButton;
  QLineEdit* CustomRequestLineEdit;

  // Отображение записей в БД
  QGroupBox* DatabaseBufferGroup;
  QVBoxLayout* DatabaseBufferLayout;
  QTableView* DatabaseRandomBufferView;
  //============================================================

  /* Интерфейс для упралвения заказами */
  //============================================================
  QWidget* OrderTab;
  QHBoxLayout* OrderTabMainLayout;

  QGroupBox* OrderControlPanel;
  QVBoxLayout* OrderControlPanelLayout;

  QCheckBox* FullPersonalizationCheckBox;

  QWidget* OrderPanelSubWidget;
  QHBoxLayout* OrderPanelSubLayout;
  QLabel* PanFilePathLabel;
  QLineEdit* PanFilePathLineEdit;
  QPushButton* PanFileExplorePushButton;

  QHBoxLayout* OrderPanelSubLayout1;
  QLabel* IssuerNameComboLabel;
  QComboBox* IssuerNameComboBox;

  QHBoxLayout* OrderPanelSubLayout2;
  QLabel* TransponderQuantityLabel;
  QLineEdit* TransponderQuantityLineEdit;

  QHBoxLayout* OrderPanelSubLayout3;
  QLabel* BoxCapacityLabel;
  QLineEdit* BoxCapacityLineEdit;

  QHBoxLayout* OrderPanelSubLayout4;
  QLabel* PalletCapacityLabel;
  QLineEdit* PalletCapacityLineEdit;

  QPushButton* CreateNewOrderPushButton;
  QSpacerItem* OrderControlPanelVS;
  QPushButton* UpdateOrderViewPushButton;
  QPushButton* DeleteLastOrderPushButton;

  QGroupBox* OrderTablePanel;
  QVBoxLayout* OrderTablePanelLayout;
  QTableView* OrderTableView;
  //============================================================

  /* Интерфейс для управления линиями производства */
  //============================================================
  QWidget* ProductionLinesTab;
  QHBoxLayout* ProductionLinesTabMainLayout;

  QGroupBox* ProductionLinesControlPanel;
  QVBoxLayout* ProductionLinesControlPanelLayout;

  QHBoxLayout* LoginLayout;
  QLabel* LoginLabel;
  QLineEdit* LoginLineEdit;

  QHBoxLayout* PasswordLayout;
  QLabel* PasswordLabel;
  QLineEdit* PasswordLineEdit;

  QPushButton* CreateNewProductionLinePushButton;
  QSpacerItem* ProductionLinesControlPanelVS;
  QPushButton* UpdateProductionLineViewPushButton;
  QPushButton* DeleteLastProductionLinePushButton;

  QGroupBox* ProductionLineTablePanel;
  QVBoxLayout* ProductionLineTableLayout;
  QTableView* ProductionLineTableView;
  //============================================================

  /* Интерфейс для управления линиями производства */
  //============================================================
  QWidget* TransponderTab;
  QHBoxLayout* TransponderTabMainLayout;

  QGroupBox* TransponderControlPanel;
  QVBoxLayout* TransponderControlPanelLayout;

  QHBoxLayout* UcidLayout;
  QLabel* UcidLabel;
  QLineEdit* UcidLineEdit;
  QPushButton* ReleaseTransponderPushButton;

  QSpacerItem* TransponderControlPanelVS;

  QHBoxLayout* SearchByLayout;
  QLabel* SearchByLabel;
  QComboBox* SearchByComboBox;

  QHBoxLayout* SearchInputLayout;
  QLabel* SearchInputLabel;
  QLineEdit* SearchInputLineEdit;
  QPushButton* SearchPushButton;
  QPushButton* RevokeTransponderPushButton;
  QPushButton* RereleaseTransponderPushButton;

  QGroupBox* TransponderDisplayPanel;
  QHBoxLayout* TransponderDisplayLayout;
  QListView* TransponderDataListView;
  QPlainTextEdit* FirmwareDsrcDataView;
  //============================================================

  /* Интерфейс транспортных ключей безопасности */
  //============================================================
  QWidget* TransportKeyTab;
  QHBoxLayout* TransportKeyMainLayout;

  // Панель управления
  QGroupBox* TransportKeyControlPanelGroup;
  QVBoxLayout* TransportKeyControlPanelLayout;

  QPushButton* UpdateTransportKeyPushButton;
  QSpacerItem* TransportKeyVS1;
  QPushButton* ClearTransportKeyPushButton;

  // Отображение записей
  QGroupBox* TransportKeyViewGroup;
  QVBoxLayout* TransportKeyViewLayout;
  QTableView* TransportKeyView;
  //============================================================

  /* Интерфейс коммерческих ключей безопасности */
  //============================================================
  QWidget* CommercialKeyTab;
  QHBoxLayout* CommercialKeyMainLayout;

  // Панель управления
  QGroupBox* CommercialKeyControlPanelGroup;
  QVBoxLayout* CommercialKeyControlPanelLayout;

  QPushButton* UpdateCommercialKeyPushButton;
  QSpacerItem* CommercialKeyVS1;
  QPushButton* ClearCommercialKeyPushButton;

  // Отображение записей
  QGroupBox* CommercialKeyViewGroup;
  QVBoxLayout* CommercialKeyViewLayout;
  QTableView* CommercialKeyView;
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
  QLabel* DatabaseLogOptionLabel;
  QCheckBox* DatabaseLogOption;

  // Настройки генератора прошивок
  QGroupBox* FirmwareSettingsGroupBox;
  QGridLayout* FirmwareSettingsLayout;
  QLabel* FirmwareFilePathLabel;
  QLineEdit* FirmwareFilePathLineEdit;
  QPushButton* ExploreFirmwareFilePathPushButton;
  QLabel* TransponderDataFilePathLabel;
  QLineEdit* TransponderDataFilePathLineEdit;
  QPushButton* ExploreTransponderDataFilePathPushButton;
  //============================================================
 public:
  explicit MasterGUI(QWidget* parent);

  virtual void create(void) override;
  virtual void update(void) override;

 private:
  void createTabs(void);

  void createServerTab(void);
  void createDatabaseTab(void);
  void createOrderTab(void);
  void createProductionLineTab(void);
  void createFirmwareTab(void);
  void createTransportKeyTab(void);
  void createCommercialKeyTab(void);

  void createSettingsTab(void);

 private slots:
  void on_FullPersonalizationCheckBoxChanged(void);
  void on_PanFileExplorePushButton_slot(void);

  void on_ExploreFirmwareFilePathPushButton_slot(void);
  void on_ExploreTransponderDataFilePathPushButton_slot(void);

 signals:
};

#endif  // GUI_MASTER_H
