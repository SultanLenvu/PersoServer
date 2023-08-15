#include "gui_initial.h"

GUI_Initial::GUI_Initial(QObject* parent) : GUI(parent, InitialConfiguration) {
  setObjectName("GUI_Initial");
}

QWidget* GUI_Initial::create() {
  // Создаем панель управления
  createInitialMenu();

  // Создаем виджеты для отображения логов
  createLog();

  // Настраиваем пропорции
  MainLayout->setStretch(0, 1);
  MainLayout->setStretch(1, 3);

  return MainWidget;
}

void GUI_Initial::update() {}

void GUI_Initial::createInitialMenu() {
  InitialMenuGroup = new QGroupBox("Панель управления");
  InitialMenuGroup->setAlignment(Qt::AlignCenter);
  MainLayout->addWidget(InitialMenuGroup);

  InitialMenuLayout = new QVBoxLayout();
  InitialMenuGroup->setLayout(InitialMenuLayout);

  StartServerPushButton = new QPushButton("Запустить сервер");
  InitialMenuLayout->addWidget(StartServerPushButton);
  StopServerPushButton = new QPushButton("Остановить сервер");
  InitialMenuLayout->addWidget(StopServerPushButton);
  OpenMasterPushButton = new QPushButton("Открыть мастер интерфейс");
  InitialMenuLayout->addWidget(OpenMasterPushButton);

  ConnectButtonSpacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  InitialMenuLayout->addItem(ConnectButtonSpacer);
}
