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
  QSpacerItem* PushButtonLayoutVS1;

  QPushButton* TransmitCustomRequestPushButton;
  QLineEdit* CustomRequestLineEdit;

  // Отображение записей в БД
  QGroupBox* DatabaseBufferGroup;
  QVBoxLayout* DatabaseBufferLayout;
  QTableView* DatabaseBufferView;
  //============================================================

  /* Интерфейс таблицы линий производства*/
  //============================================================
  QWidget* ProductionLineTab;
  QHBoxLayout* ProductionLineMainLayout;

  // Панель упралвения
  QGroupBox* ProductionLineControlPanelGroup;
  QVBoxLayout* ProductionLineControlPanelLayout;

  QPushButton* UpdateProductionLinePushButton;
  QSpacerItem* ProductionLineVS1;
  QPushButton* ClearProductionLinePushButton;

  // Отображение записей
  QGroupBox* ProductionLineViewGroup;
  QVBoxLayout* ProductionLineViewLayout;
  QTableView* ProductionLineView;
  //============================================================

  /* Интерфейс таблицы транспонедров  */
  //============================================================
  QWidget* TransponderTab;
  QHBoxLayout* TransponderMainLayout;

  // Панель упралвения
  QGroupBox* TransponderControlPanelGroup;
  QVBoxLayout* TransponderControlPanelLayout;

  QPushButton* UpdateTransponderPushButton;
  QSpacerItem* TransponderVS1;
  QPushButton* ClearTransponderPushButton;

  // Отображение записей
  QGroupBox* TransponderViewGroup;
  QVBoxLayout* TransponderViewLayout;
  QTableView* TransponderView;
  //============================================================

  /* Интерфейс заказов */
  //============================================================
  QWidget* OrderTab;
  QHBoxLayout* OrderTabMainLayout;
  QVBoxLayout* OrderTabPanelLayout;

  QGroupBox* OrderControlPanel;
  QVBoxLayout* OrderControlPanelLayout;

  QPushButton* UpdateOrderPushButton;
  QSpacerItem* OrderControlPanelVS1;
  QPushButton* ClearOrderPushButton;

  QGroupBox* OrderCreationPanel;
  QVBoxLayout* OrderCreationPanelLayout;

  QCheckBox* FullPersonalizationCheckBox;

  QWidget* OrderCreationPanelSubWidget;
  QHBoxLayout* OrderCreationPanelSubLayout;
  QLabel* PanFilePathLabel;
  QLineEdit* PanFilePathLineEdit;
  QPushButton* PanFileExplorePushButton;

  QHBoxLayout* OrderCreationPanelSubLayout1;
  QLabel* IssuerNameComboLabel;
  QComboBox* IssuerNameComboBox;

  QHBoxLayout* OrderCreationPanelSubLayout2;
  QLabel* TransponderQuantityLabel;
  QLineEdit* TransponderQuantityLineEdit;

  QPushButton* CreateNewOrderPushButton;
  QSpacerItem* OrderCreationPanelVS;

  QGroupBox* PanListPanel;
  QVBoxLayout* PanListLayout;
  QTableView* PanListTableView;

  QSpacerItem* OrderTabMainLayoutHS;
  //============================================================

  /* Интерфейс таблицы эмитентов  */
  //============================================================
  QWidget* IssuerTab;
  QHBoxLayout* IssuerMainLayout;

  // Панель управления
  QGroupBox* IssuerControlPanelGroup;
  QVBoxLayout* IssuerControlPanelLayout;

  QPushButton* UpdateIssuerPushButton;
  QSpacerItem* IssuerVS1;
  QPushButton* ClearIssuerPushButton;

  // Отображение записей
  QGroupBox* IssuerViewGroup;
  QVBoxLayout* IssuerViewLayout;
  QTableView* IssuerView;
  //============================================================

  /* Интерфейс таблицы боксов  */
  //============================================================
  QWidget* BoxTab;
  QHBoxLayout* BoxMainLayout;

  // Панель упралвения
  QGroupBox* BoxControlPanelGroup;
  QVBoxLayout* BoxControlPanelLayout;

  QPushButton* UpdateBoxPushButton;
  QSpacerItem* BoxVS1;
  QPushButton* ClearBoxPushButton;

  // Отображение записей
  QGroupBox* BoxViewGroup;
  QVBoxLayout* BoxViewLayout;
  QTableView* BoxView;
  //============================================================

  /* Интерфейс таблицы палет  */
  //============================================================
  QWidget* PalletTab;
  QHBoxLayout* PalletMainLayout;

  // Панель управления
  QGroupBox* PalletControlPanelGroup;
  QVBoxLayout* PalletControlPanelLayout;

  QPushButton* UpdatePalletPushButton;
  QSpacerItem* PalletVS1;
  QPushButton* ClearPalletPushButton;

  // Отображение записей
  QGroupBox* PalletViewGroup;
  QVBoxLayout* PalletViewLayout;
  QTableView* PalletView;
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
  //============================================================
 public:
  explicit MasterGUI(QWidget* parent);

  virtual void create(void) override;
  virtual void update(void) override;

 private:
  void createTabs(void);

  void createServerTab(void);
  void createDatabaseTab(void);
  void createProductionLineTab(void);
  void createTransponderTab(void);
  void createOrderTab(void);
  void createIssuerTab(void);
  void createBoxTab(void);
  void createPalletTab(void);
  void createTransportKeyTab(void);
  void createCommercialKeyTab(void);

  void createSettingsTab(void);

 private slots:
  void on_FullPersonalizationCheckBoxChanged(void);
  void on_PanFileExplorePushButton_slot(void);

 signals:
};

#endif  // GUI_MASTER_H
