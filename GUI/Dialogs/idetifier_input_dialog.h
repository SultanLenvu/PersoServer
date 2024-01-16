#ifndef IdentifierInputDialog_H
#define IdentifierInputDialog_H

#include "abstract_input_dialog.h"

class IdentifierInputDialog : public AbstractInputDialog {
  Q_OBJECT
 private:
  QSize DesktopGeometry;

  QGridLayout* MainLayout;

  QLabel* MainLabel;
  QLineEdit* InputData;

  QHBoxLayout* ButtonLayout;
  QPushButton* AcceptButton;
  QPushButton* RejectButton;

 public:
  explicit IdentifierInputDialog(QWidget* parent);
  ~IdentifierInputDialog();

  virtual void getData(QHash<QString, QString>* data) const override;
  virtual InputDialogType type(void) const override;

  virtual void accept() override;

 private:
  Q_DISABLE_COPY_MOVE(IdentifierInputDialog);
  void create(void);
  bool check(void) const;
};

#endif  // IdentifierInputDialog_H
