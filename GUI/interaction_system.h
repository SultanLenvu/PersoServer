#ifndef INTERACTION_SYSTEM_H
#define INTERACTION_SYSTEM_H

#include <QElapsedTimer>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QProgressBar>
#include <QProgressDialog>
#include <QSettings>
#include <QTimer>

#include "Dialogs/abstract_input_dialog.h"

class InteractionSystem : public QWidget {
  Q_OBJECT

 private:
  bool LogEnable;
  QProgressDialog ProgressDialog;
  uint32_t CurrentOperationStep;

  std::unique_ptr<AbstractInputDialog> CurrentDialog;

  QTimer* ODTimer;
  QTimer* ODQTimer;
  QElapsedTimer* ODMeter;

 public:
  static InteractionSystem* instance(void);

 public slots:
  void generateMessage(const QString& pass);
  void getMasterPassword(QString& pass);
  void generateErrorMessage(const QString& text);

  void startOperationProgressDialog(const QString& operationName);
  void finishOperationProgressDialog(const QString& operationName);

  void applySettings(void);

  void invalidInput_slot(void);

 private:
  explicit InteractionSystem(QWidget* window);
  Q_DISABLE_COPY_MOVE(InteractionSystem)

 private:
  void loadSettings(void);
  void sendLog(const QString& log);

  void createProgressDialog(void);
  void destroyProgressDialog(void);
  void createTimers(void);
  void processCurrentDialog(QHash<QString, QString>* param);

 private slots:
  void progressDialogCanceled_slot(void);

  void ODTimerTimeout_slot(void);
  void ODQTimerTimeout_slot(void);

 signals:
  void logging(const QString& log);
  void abortCurrentOperation(void);
};

#endif  // INTERACTION_SYSTEM_H
