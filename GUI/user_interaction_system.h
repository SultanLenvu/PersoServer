#ifndef USER_INTERACTION_SYSTEM_H
#define USER_INTERACTION_SYSTEM_H

#include <QDebug>
#include <QDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QtWidgets>

#include "General/definitions.h"
#include "perso_init_dialog.h"

/* Система оповещения пользователя */
//==================================================================================

class UserInteractionSystem : public QWidget {
  Q_OBJECT
 private:
  QWidget* ParentWindow;
  QDialog* CurrentDialog;

 public:
  UserInteractionSystem(QWidget* parent, QWidget* window);

 public slots:
  void generateNotification(const QString& data);
  void generateError(const QString& data);
  void getUserInputKey(QString& key);

  void getPathToFile(QString& data);

 private:
  void createPersoInitWindow(void);
};

//==================================================================================

#endif  // USER_INTERACTION_SYSTEM_H
