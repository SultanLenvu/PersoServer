#ifndef GUI_MASTER_H
#define GUI_MASTER_H

#include <QObject>
#include <QtCharts>

#include "gui.h"
#include "gui_delegates.h"

#include "General/definitions.h"

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
  QSpacerItem* DatabaseControlPanelVS1;

  QPushButton* TransmitCustomRequestPushButton;
  QLineEdit* CustomRequestLineEdit;

  // Отображение записей в БД
  QGroupBox* DatabaseBufferGroup;
  QVBoxLayout* DatabaseBufferLayout;
  QTableView* DatabaseRandomModelView;
  //============================================================

  /* Интерфейс для управления заказами */
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
  QHBoxLayout* OrderPanelSublayout4;
  QLabel* PalletCapacityLabel;
  QLineEdit* PalletCapacityLineEdit;
  QHBoxLayout* OrderPanelSublayout5;
  QLabel* TransponderModelLabel;
  QLineEdit* TransponderModelLineEdit;
  QHBoxLayout* AccrReferenceSublayout6;
  QLabel* AccrReferenceLabel;
  QLineEdit* AccrReferenceLineEdit;
  QPushButton* CreateNewOrderPushButton;
  QSpacerItem* OrderControlPanelVS1;

  QHBoxLayout* OrderIdLayout1;
  QLabel* OrderIdLabel1;
  QLineEdit* OrderIdLineEdit1;
  QPushButton* StartOrderAssemblingPushButton;
  QPushButton* StopOrderAssemblingPushButton;
  QSpacerItem* OrderControlPanelVS2;

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

  QHBoxLayout* LoginLayout1;
  QLabel* LoginLabel1;
  QLineEdit* LoginLineEdit1;
  QHBoxLayout* PasswordLayout1;
  QLabel* PasswordLabel1;
  QLineEdit* PasswordLineEdit1;
  QPushButton* CreateNewProductionLinePushButton;
  QSpacerItem* ProductionLinesControlPanelVS1;

  QHBoxLayout* OrderIdLayout2;
  QLabel* OrderIdLabel2;
  QLineEdit* OrderIdLineEdit2;
  QPushButton* AllocateInactiveProductionLinesPushButton;
  QSpacerItem* ProductionLinesControlPanelVS2;

  QHBoxLayout* BoxIdLayout;
  QLabel* BoxIdLabel;
  QLineEdit* BoxIdLineEdit;
  QPushButton* LinkProductionLinePushButton;
  QSpacerItem* ProductionLinesControlPanelVS3;

  QPushButton* DeactivateAllProductionLinesPushButton;
  QPushButton* UpdateProductionLineViewPushButton;
  QPushButton* DeleteLastProductionLinePushButton;

  QGroupBox* ProductionLineTablePanel;
  QVBoxLayout* ProductionLineTableLayout;
  QTableView* ProductionLineTableView;
  //============================================================

  /* Интерфейс для управления транспондерами */
  //============================================================
  QWidget* TransponderTab;
  QHBoxLayout* TransponderTabMainLayout;

  QGroupBox* TransponderControlPanel;
  QVBoxLayout* TransponderControlPanelLayout;

  QHBoxLayout* LoginLayout2;
  QLabel* LoginLabel2;
  QLineEdit* LoginLineEdit2;
  QHBoxLayout* PasswordLayout2;
  QLabel* PasswordLabel2;
  QLineEdit* PasswordLineEdit2;
  QHBoxLayout* UcidLayout;
  QLabel* UcidLabel;
  QLineEdit* UcidLineEdit;
  QPushButton* ReleaseTransponderPushButton;
  QPushButton* ConfirmTransponderPushButton;

  QSpacerItem* TransponderControlPanelVS;

  QHBoxLayout* SearchTransponderByLayout;
  QComboBox* SearchTransponderByComboBox;
  QLineEdit* SearchTransponderLineEdit;
  QPushButton* SearchTransponderPushButton;
  QPushButton* RefundTransponderPushButton;

  QHBoxLayout* LoginLayout3;
  QLabel* LoginLabel3;
  QLineEdit* LoginLineEdit3;
  QHBoxLayout* PasswordLayout3;
  QLabel* PasswordLabel3;
  QLineEdit* PasswordLineEdit3;
  QHBoxLayout* RereleaseTransponderLayout;
  QComboBox* RereleaseTransponderByComboBox;
  QLineEdit* RereleaseTransponderLineEdit;
  QHBoxLayout* NewUcidLayout;
  QLabel* NewUcidLabel;
  QLineEdit* NewUcidLineEdit;
  QPushButton* RereleaseTransponderPushButton;
  QPushButton* ConfirmRereleaseTransponderPushButton;

  QGroupBox* TransponderDisplayPanel;
  QVBoxLayout* TransponderDisplayLayout;
  QTableView* TransponderSeedTableView;
  QPlainTextEdit* AssembledFirmwareView;
  //============================================================

  /* Интерфейс для управления эмитентами */
  //============================================================
  QWidget* IssuerTab;
  QHBoxLayout* IssuerTabMainLayout;

  // Панель управления
  QGroupBox* IssuerControlPanelGroup;
  QVBoxLayout* IssuerControlPanelLayout;

  QComboBox* IssuerTableChoice;
  QPushButton* ShowIssuerTablePushButton;
  QPushButton* InitTransportMasterKeysPushButton;
  QPushButton* InitIssuerTablePushButton;
  QSpacerItem* TransportKeyVS1;

  QHBoxLayout* IssuerIdLayout1;
  QLabel* IssuerIdLabel1;
  QLineEdit* IssuerIdLineEdit1;
  QHBoxLayout* MasterKeysIdLayout1;
  QComboBox* MasterKeysChoice;
  QLineEdit* MasterKeysLineEdit1;
  QPushButton* LinkIssuerWithKeysPushButton;

  // Отображение записей
  QGroupBox* IssuerViewGroup;
  QVBoxLayout* IssuerTableViewLayout;
  QTableView* IssuerTableView;
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
  QLabel* FirmwareBasePathLabel;
  QLineEdit* FirmwareBasePathLineEdit;
  QPushButton* ExploreFirmwareBasePathPushButton;
  QLabel* FirmwareDataPathLabel;
  QLineEdit* FirmwareDataPathLineEdit;
  QPushButton* ExploreFirmwareDataPathPushButton;
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
  void createTransponderTab(void);
  void createIssuerTab(void);

  void createSettingsTab(void);

 private slots:
  void on_FullPersonalizationCheckBoxChanged_slot(void);
  void on_PanFileExplorePushButton_slot(void);

  void on_ExploreFirmwareBasePathPushButton_slot(void);
  void on_ExploreFirmwareDataPathPushButton_slot(void);

  void on_SearchTransponderByComboBox_slot(const QString& text);
  void on_RereleaseTransponderByComboBox_slot(const QString& text);

 signals:
};

#endif  // GUI_MASTER_H
