#include "gui_initial.h"

InitialGUI::InitialGUI(QWidget* parent) : GUI(parent, InitialConfiguration) {
  setObjectName("InitialGUI");
}

void InitialGUI::create() {
  // Создаем панель управления
  createInitialMenu();

  // Создаем виджеты для отображения логов
  createLog();

  // Настраиваем пропорции
  MainLayout->setStretch(0, 1);
  MainLayout->setStretch(1, 3);
}

void InitialGUI::update() {}

void InitialGUI::createInitialMenu() {
  InitialMenuGroup = new QGroupBox("Панель управления");
  InitialMenuGroup->setAlignment(Qt::AlignCenter);
  MainLayout->addWidget(InitialMenuGroup);

  InitialMenuLayout = new QVBoxLayout();
  InitialMenuGroup->setLayout(InitialMenuLayout);

  StartServerPushButton = new QPushButton("Запустить сервер");
  InitialMenuLayout->addWidget(StartServerPushButton);
  StopServerPushButton = new QPushButton("Остановить сервер");
  InitialMenuLayout->addWidget(StopServerPushButton);
  OpenMasterGuiPushButton = new QPushButton("Открыть мастер интерфейс");
  InitialMenuLayout->addWidget(OpenMasterGuiPushButton);

  ConnectButtonSpacer =
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  InitialMenuLayout->addItem(ConnectButtonSpacer);
}
