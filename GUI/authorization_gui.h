#ifndef AbstractGUI_AUTHORIZATION_H
#define AbstractGUI_AUTHORIZATION_H

#include <QObject>
#include <QSettings>
#include <QtWidgets>

#include "General/definitions.h"
#include "abstract_gui.h"

class AuthorizationGUI : public AbstractGUI {
  Q_OBJECT
 public:
  QGroupBox* AuthorizationMenuGroup;
  QGridLayout* AuthorizationMenuLayout;

  QLabel* LoginLabel;
  QLineEdit* LoginLineEdit;

  QLabel* PasswordLabel;
  QLineEdit* PasswordLineEdit;

  QPushButton* AuthorizePushButton;

  QSpacerItem* ConnectButtonSpacer;

 public:
  explicit AuthorizationGUI(QWidget* parent);

  virtual void create(void) override;
  virtual void update(void) override;

 private:
  Q_DISABLE_COPY_MOVE(AuthorizationGUI)
  void createAuthorizationMenu(void);

 private slots:
};

#endif  // AbstractGUI_AUTHORIZATION_H
