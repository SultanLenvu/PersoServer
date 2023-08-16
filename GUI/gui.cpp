#include "gui.h"

GUI::GUI(QObject* parent, GUI::GUI_Type type) : QObject(parent)
{
  MainWidget = new QWidget();

  MainLayout = new QHBoxLayout();
  MainWidget->setLayout(MainLayout);

  Type = type;
}

GUI::~GUI()
{
  QObject::disconnect(this);
  delete MainWidget;
}

GUI::GUI_Type GUI::type()
{
  return Type;
}

void GUI::displayLog(const QString& data) {
  if (LogDisplay->toPlainText().length() > 500000)
    LogDisplay->clear();

  if (LogDisplay)
    LogDisplay->appendPlainText(data);
}

void GUI::clearLogDisplay() {
  LogDisplay->clear();
}

void GUI::createLog() {
  LogGroup = new QGroupBox("Лог");
  LogGroup->setAlignment(Qt::AlignCenter);
  MainLayout->addWidget(LogGroup);

  LogLayout = new QVBoxLayout();
  LogGroup->setLayout(LogLayout);

  LogDisplay = new QPlainTextEdit();
  LogDisplay = new QPlainTextEdit();
  LogDisplay->setEnabled(true);
  LogDisplay->setTabletTracking(true);
  LogDisplay->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  LogDisplay->setCenterOnScroll(false);
  LogLayout->addWidget(LogDisplay);
}
