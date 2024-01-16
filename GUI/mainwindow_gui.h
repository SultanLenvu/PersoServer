#ifndef MAINWINDOW_GUI_H
#define MAINWINDOW_GUI_H

#include <QObject>
#include <QtCharts>

#include "abstract_gui.h"

class MainWindowGUI : public AbstractGUI {
  Q_OBJECT

 public:
  QTabWidget* Tabs;

  /* Отображение логов */
  //============================================================
  QGroupBox* LogGroup;
  QVBoxLayout* LogLayout;
  QPlainTextEdit* LogDisplay;
  //============================================================

  /* Интерфейс базы данных */
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

  QPushButton* CreateNewOrderPushButton;
  QPushButton* StartOrderAssemblingPushButton;
  QPushButton* StopOrderAssemblingPushButton;
  QSpacerItem* OrderControlPanelVS1;

  QPushButton* UpdateOrderViewPushButton;

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

  QPushButton* CreateNewProductionLinePushButton;
  QPushButton* StartProductionLinePushButton;
  QPushButton* StopProductionLinePushButton;
  QPushButton* ShutdownAllProductionLinesPushButton;
  QPushButton* EditProductionLinesPushButton;
  QSpacerItem* ProductionLinesControlPanelVS2;

  QPushButton* UpdateProductionLineViewPushButton;

  QGroupBox* ProductionLineTablePanel;
  QVBoxLayout* ProductionLineTableLayout;
  QTableView* ProductionLineTableView;
  //============================================================

  /* Интерфейс для теста сервера */
  //============================================================
  QWidget* ServerTab;
  QHBoxLayout* ServerTabMainLayout;

  QGroupBox* ServerTabControlPanel;
  QVBoxLayout* ServerTabControlPanelLayout;

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

  QHBoxLayout* RereleaseKeyLayout;
  QComboBox* RereleaseKeyComboBox;
  QLineEdit* RereleaseKeyLineEdit;
  QPushButton* RereleaseTransponderPushButton;
  QPushButton* ConfirmRereleaseTransponderPushButton;
  QPushButton* ProductionLineRollbackPushButton;
  QSpacerItem* ServerTabControlPanelVS;

  QPushButton* PrintBoxStickerOnServerPushButton;
  QPushButton* PrintLastBoxStickerOnServerPushButton;
  QPushButton* PrintPalletStickerOnServerPushButton;
  QPushButton* PrintLastPalletStickerOnServerPushButton;

  QGroupBox* TransponderDisplayPanel;
  QVBoxLayout* TransponderDisplayLayout;
  QTableView* TransponderDataTableView;
  QPlainTextEdit* AssembledFirmwareView;
  //============================================================

  /* Интерфейс для управления транспондерами */
  //============================================================
  QWidget* TransponderTab;
  QHBoxLayout* TransponderTabMainLayout;

  QGroupBox* TransponderControlPanel;
  QVBoxLayout* TransponderControlPanelLayout;

  QPushButton* TransponderManualReleasePushButton;
  QPushButton* TransponderManualRefundPushButton;
  QSpacerItem* TransponderControlPanelLayoutVS;

  QPushButton* PalletShipmentPushButton;

  QGroupBox* TransponderViewGroup;
  QVBoxLayout* TransponderViewGroupLayout;
  QTableView* TransponderTableView;
  //============================================================

  /* Интерфейс для управления эмитентами */
  //============================================================
  QWidget* IssuerTab;
  QHBoxLayout* IssuerTabMainLayout;

  // Панель управления
  QGroupBox* IssuerControlPanelGroup;
  QVBoxLayout* IssuerControlPanelLayout;

  QPushButton* InitTransportMasterKeysPushButton;
  QPushButton* InitIssuerTablePushButton;
  QPushButton* LinkIssuerWithKeysPushButton;
  QSpacerItem* TransportKeyVS1;

  // Отображение записей
  QGroupBox* IssuerViewGroup;
  QVBoxLayout* IssuerTableViewLayout;
  QTableView* IssuerTableView;
  //============================================================

  /* Принтер стикеров */
  //============================================================
  QWidget* StickerTab;
  QHBoxLayout* StickerMainLayout;

  QGroupBox* StickerControlPanel;
  QVBoxLayout* StickerControlPanelLayout;

  QPushButton* PrintTransponderStickerPushButton;
  QPushButton* PrintBoxStickerPushButton;
  QPushButton* PrintPalletStickerPushButton;
  QSpacerItem* StickerControlPanelVS;
  QPushButton* ExecStickerPrinterCommandScriptPushButton;

  QGroupBox* StickerDataViewGroup;
  QVBoxLayout* StickerDataViewLayout;
  QTableView* StickerDataTableView;
  QPlainTextEdit* StickerPrinterCommandScriptInput;
  //============================================================

 public:
  explicit MainWindowGUI(QWidget* parent);

  virtual void create(void) override;
  virtual void update(void) override;

 public slots:
  void displayLog(const QString& data);
  void clearLogDisplay(void);

 private:
  Q_DISABLE_COPY_MOVE(MainWindowGUI)
  void createTabs(void);

  void createDatabaseTab(void);
  void createOrderTab(void);
  void createProductionLineTab(void);
  void createServerTab(void);
  void createTransponderTab(void);
  void createIssuerTab(void);
  void createStickerTab(void);

  void createLog(void);

 private slots:
  void rereleaseKeyComboBox_slot(const QString& text);

 signals:
};

#endif  // MAINWINDOW_GUI_H
