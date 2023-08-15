#ifndef PERSOINITDIALOG_H
#define PERSOINITDIALOG_H

#include <QDialog>
#include <QtWidgets>

class PersoInitDialog : public QDialog {
  Q_OBJECT

 private:
  QRect DesktopGeometry;

  QVBoxLayout* MainLayout;
  QLabel* MainLabel;
  QPlainTextEdit* PersoInitDataInput;
  QPushButton* OkButton;
  QPushButton* CancelButton;

 public:
  PersoInitDialog(QWidget* parent);
  QString getData(void);
};

#endif  // PERSOINITDIALOG_H
