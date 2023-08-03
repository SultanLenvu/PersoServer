#ifndef GUI_MASTER_H
#define GUI_MASTER_H

#include <QObject>
#include <QtCharts>

#include "gui.h"
#include "gui_delegates.h"

class GUI_Master : public GUI {
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
  QPushButton* TransmitCustomRequestPushButton;
  QLineEdit* CustomRequestLineEdit;
  QSpacerItem* PushButtonLayoutVS;

  // Отображение записей в БД
  QGroupBox* DataBaseBufferGroup;
  QVBoxLayout* DataBaseBufferLayout;
  QTableView* DataBaseBufferView;
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

  // Персонализация
  QGroupBox* PersoSettingsGroupBox;
  QGridLayout* PersoSettingsMainLayout;
  QLabel* UseServerPersoLabel;
  QCheckBox* UseServerPersoCheckBox;
  QLabel* ServerCommonKeyGenerationLabel;
  QCheckBox* ServerCommonKeyGenerationCheckBox;
  QLabel* PersoServerIpAddressLabel;
  QLineEdit* PersoServerIpAddressLineEdit;
  QLabel* PersoServerPortLabel;
  QLineEdit* PersoServerPortLineEdit;
  QLabel* localMasterKeyPathLabel;
  QLineEdit* localMasterKeyPathLineEdit;
  //============================================================
 public:
  explicit GUI_Master(QObject* parent);

  virtual QWidget* create(void) override;
  virtual void update(void) override;

 private:
  void createTabs(void);
  void createDataBaseTab(void);
  void createSecurityTab(void);
  void createSettingsTab(void);

 signals:
};

#endif  // GUI_MASTER_H
