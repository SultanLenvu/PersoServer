#ifndef SETTINGSINPUTDIALOG_H
#define SETTINGSINPUTDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QHostAddress>
#include <QtWidgets>

class SettingsDialog : public QDialog {
  Q_OBJECT
 private:
  QSettings Settings;

  QSize DesktopGeometry;

  QVBoxLayout* MainLayout;

  // Настройки базы данных
  QGroupBox* DatabaseGroupBox;
  QGridLayout* DatabaseLayout;
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

  // Настройки клиента
  QGroupBox* PersoClientGroupBox;
  QGridLayout* PersoClientMainLayout;
  QLabel* PersoClientServerIdLabel;
  QLineEdit* PersoClientServerIpLineEdit;
  QLabel* PersoClientServerPortLabel;
  QLineEdit* PersoClientServerPortLineEdit;

  // Настройки системы логгирования
  QGroupBox* LogSystemGroupBox;
  QGridLayout* LogSystemLayout;
  QLabel* LogSystemGlobalEnableLabel;
  QCheckBox* LogSystemGlobalEnableCheckBox;

  QLabel* LogSystemExtendedEnableLabel;
  QCheckBox* LogSystemExtendedEnableCheckBox;

  QWidget* LogSystemProxyWidget1;
  QGridLayout* LogSystemProxyWidget1Layout;

  QLabel* LogSystemDisplayEnableLabel;
  QCheckBox* LogSystemDisplayEnableCheckBox;

  QLabel* LogSystemListenPersoServerLabel;
  QCheckBox* LogSystemListenPersoServerCheckBox;

  QWidget* LogSystemProxyWidget2;
  QGridLayout* LogSystemProxyWidget2Layout;
  QLabel* LogSystemListenIpLabel;
  QLineEdit* LogSystemListenIpLineEdit;
  QLabel* LogSystemListenPortLabel;
  QLineEdit* LogSystemListenPortLineEdit;

  QLabel* LogSystemFileEnableLabel;
  QCheckBox* LogSystemFileEnableCheckBox;

  QWidget* LogSystemProxyWidget3;
  QGridLayout* LogSystemProxyWidget3Layout;
  QLabel* LogSystemFileMaxNumberLabel;
  QLineEdit* LogSystemFileMaxNumberLineEdit;

  // Настройки принтера
  QGroupBox* StickerPrinterGroupBox;
  QGridLayout* StickerPrinterMainLayout;
  QLabel* StickerPrinterLibPathLabel;
  QLineEdit* StickerPrinterLibPathLineEdit;
  QPushButton* StickerPrinterLibPathPushButton;
  QLabel* StickerPrinterNameLabel;
  QLineEdit* StickerPrinterNameLineEdit;

  // Кнопки
  QHBoxLayout* ButtonLayout;
  QPushButton* ApplyPushButton;
  QPushButton* RejectPushButton;

 public:
  explicit SettingsDialog(QWidget* parent);
  ~SettingsDialog();

  virtual void accept(void) override;

 private:
  Q_DISABLE_COPY_MOVE(SettingsDialog)
  void create(void);
  bool check() const;
  void save(void);

 private slots:
  void logSystemEnableCheckBox_slot(int32_t state);
  void logSystemListenPersoServerCheckBox_slot(int32_t state);
  void logSystemFileEnableCheckBox_slot(int32_t state);
  void stickerPrinterLibPathPushButton_slot(void);

 signals:
  void applyNewSettings();
};

#endif  // SETTINGSINPUTDIALOG_H
