#ifndef GUI_INITIAL_H
#define GUI_INITIAL_H

#include <QObject>
#include <QtWidgets>

#include "gui.h"

class InitialGUI : public GUI {
  Q_OBJECT
 public:
  QGroupBox* InitialMenuGroup;
  QVBoxLayout* InitialMenuLayout;
  QPushButton* StartServerPushButton;
  QPushButton* StopServerPushButton;
  QPushButton* OpenMasterGuiPushButton;
  QSpacerItem* ConnectButtonSpacer;

 public:
  explicit InitialGUI(QWidget* parent);

  virtual void create(void) override;
  virtual void update(void) override;

 private:
  void createInitialMenu(void);

 private slots:
};

#endif  // GUI_INITIAL_H
