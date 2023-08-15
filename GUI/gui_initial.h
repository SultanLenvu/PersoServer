#ifndef GUI_INITIAL_H
#define GUI_INITIAL_H

#include <QObject>
#include <QtWidgets>

#include "gui.h"

class GUI_Initial : public GUI {
  Q_OBJECT
 public:
  QGroupBox* InitialMenuGroup;
  QVBoxLayout* InitialMenuLayout;
  QPushButton* StartServerPushButton;
  QPushButton* StopServerPushButton;
  QPushButton* OpenMasterPushButton;
  QSpacerItem* ConnectButtonSpacer;

 public:
  explicit GUI_Initial(QObject* parent);

  virtual QWidget* create(void) override;
  virtual void update(void) override;

 private:
  void createInitialMenu(void);

 private slots:
};

#endif  // GUI_INITIAL_H
