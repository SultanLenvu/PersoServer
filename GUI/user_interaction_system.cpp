#include "user_interaction_system.h"

UserInteractionSystem::UserInteractionSystem(QObject* parent, QWidget* window)
    : QObject(parent) {
  ParentWindow = window;
  ProgressDialog = nullptr;
  CurrentOperationStep = 0;
}

void UserInteractionSystem::generateNotification(const QString& data) {
  QMessageBox::information(ParentWindow, "Сообщение", data, QMessageBox::Ok);
}

void UserInteractionSystem::getMasterPassword(QString& pass) {
  pass =
      QInputDialog::getText(ParentWindow, "Мастер доступ",
                            "Введите пароль:", QLineEdit::Normal, "", nullptr);
}

void UserInteractionSystem::generateError(const QString& text) {
  QMessageBox::critical(ParentWindow, "Ошибка", text, QMessageBox::Ok);
}

void UserInteractionSystem::generateProgressDialog(void) {
  ProgressDialog =
      new QProgressDialog("Выполнение операции...", "Закрыть", 0, 100);

  ProgressDialog->setWindowModality(Qt::ApplicationModal);
  ProgressDialog->show();
}

void UserInteractionSystem::completeProgressDialog() {
  closeProgressDialog();
}

void UserInteractionSystem::performeProgressDialogStep() {
  if (!ProgressDialog)
    return;

  CurrentOperationStep++;
  ProgressDialog->setValue(CurrentOperationStep);
}

void UserInteractionSystem::closeProgressDialog() {
  ProgressDialog->close();
  delete ProgressDialog;
  ProgressDialog = nullptr;
  CurrentOperationStep = 0;
}

void UserInteractionSystem::on_ProgressDialogCanceled_slot() {
  closeProgressDialog();

  emit abortCurrentOperation();
}
