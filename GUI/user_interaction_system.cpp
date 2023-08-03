#include "user_interaction_system.h"

UserInteractionSystem::UserInteractionSystem(QWidget* parent, QWidget* window)
    : QWidget(parent) {
  ParentWindow = window;
}

void UserInteractionSystem::generateNotification(const QString& data) {
  QMessageBox::information(ParentWindow, "Менеджер", data, QMessageBox::Ok);
}

void UserInteractionSystem::generateError(const QString& data) {
  QMessageBox::critical(ParentWindow, "Менеджер", data, QMessageBox::Ok);
}

void UserInteractionSystem::getUserInputKey(QString& key) {
  key = QInputDialog::getText(ParentWindow, "Режим работы",
                              "Введите код доступа:", QLineEdit::Normal, "",
                              nullptr);
}

void UserInteractionSystem::getPersoInitData(QString& data) {
}
