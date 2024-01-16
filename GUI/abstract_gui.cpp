#include "abstract_gui.h"

AbstractGUI::AbstractGUI(QWidget* parent, AbstractGUI::GuiType type)
    : QWidget(parent) {
  MainLayout = new QHBoxLayout();
  setLayout(MainLayout);

  Type = type;
}

AbstractGUI::~AbstractGUI()
{
}

AbstractGUI::GuiType AbstractGUI::type() {
  return Type;
}
