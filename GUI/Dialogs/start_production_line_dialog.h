#ifndef STARTPRODUCTIONLINEMENU_H
#define STARTPRODUCTIONLINEMENU_H

#include "abstract_input_dialog.h"

class StartProductionLineDialog : public AbstractInputDialog {
  Q_OBJECT
 private:
  QSize DesktopGeometry;

  QGridLayout* MainLayout;

  QLabel* ProductionLineIdLabel;
  QLineEdit* ProductionLineIdInput;

  QLabel* OrderIdLabel;
  QLineEdit* OrderIdInput;

  QHBoxLayout* ButtonLayout;
  QPushButton* AcceptButton;
  QPushButton* RejectButton;

 public:
  explicit StartProductionLineDialog(QWidget* parent);
  ~StartProductionLineDialog();

  virtual void getData(QHash<QString, QString>* data) const override;
  virtual InputDialogType type() const override;

  virtual void accept() override;

 private:
  Q_DISABLE_COPY_MOVE(StartProductionLineDialog);
  void create(void);
  bool check() const;
};

#endif  // STARTPRODUCTIONLINEMENU_H
