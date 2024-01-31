#ifndef PalletShippingDialog_H
#define PalletShippingDialog_H

#include <QDialog>
#include <QtWidgets>

#include "abstract_input_dialog.h"

class PalletShippingDialog : public AbstractInputDialog {
  Q_OBJECT
 private:
  QSize DesktopGeometry;

  QGridLayout* MainLayout;

  QLabel* PalletLabel1;
  QLineEdit* FirstPalletId;

  QLabel* PalletLabel2;
  QLineEdit* LastPalletId;

  QHBoxLayout* ButtonLayout;
  QPushButton* AcceptButton;
  QPushButton* RejectButton;

 public:
  explicit PalletShippingDialog(QWidget* parent);
  ~PalletShippingDialog();

  virtual void getData(QHash<QString, QString>* data) const override;
  virtual InputDialogType type() const override;

  virtual void accept() override;

 private:
  Q_DISABLE_COPY_MOVE(PalletShippingDialog)
  void create(void);
  bool check(void) const;
};

#endif  // PalletShippingDialog_H
