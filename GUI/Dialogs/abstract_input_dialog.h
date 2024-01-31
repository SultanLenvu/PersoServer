#ifndef AbstractInputDialog_H
#define AbstractInputDialog_H

#include <QDialog>
#include <QHash>
#include <QtWidgets>

class AbstractInputDialog : public QDialog {
  Q_OBJECT
 public:
  enum InputDialogType {
    PalletShipping,
    PanInput,
    IdentifierInput,
    ProductionLineCreation,
    StartProductionLine,
    OrderCreation,
    ManualReleaseRefund,
  };
  Q_ENUM(InputDialogType);

 public:
  explicit AbstractInputDialog(QWidget* parent);
  virtual ~AbstractInputDialog();

  virtual void getData(QHash<QString, QString>* data) const = 0;
  virtual InputDialogType type(void) const = 0;

 private:
  Q_DISABLE_COPY_MOVE(AbstractInputDialog);

 signals:
};

#endif  // AbstractInputDialog_H
