#ifndef USER_INTERACTION_SYSTEM_H
#define USER_INTERACTION_SYSTEM_H

#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QProgressBar>
#include <QProgressDialog>

class UserInteractionSystem : public QObject {
  Q_OBJECT

 private:
  QWidget* ParentWindow;
  QProgressDialog* ProgressDialog;
  uint32_t CurrentOperationStep;

 public:
  explicit UserInteractionSystem(QObject* parent, QWidget* window);

 public slots:
  void generateNotification(const QString& pass);
  void getMasterPassword(QString& pass);
  void generateError(const QString& text);
  void generateProgressDialog(void);
  void completeProgressDialog(void);
  void performeProgressDialogStep(void);

 private:
  void closeProgressDialog(void);

 private slots:
  void on_ProgressDialogCanceled_slot(void);

 signals:
  void abortCurrentOperation(void);
};

#endif  // USER_INTERACTION_SYSTEM_H
