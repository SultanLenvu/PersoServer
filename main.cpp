#include "GUI/main_window_kernel.h"

#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindowKernel w;
  w.show();
  return a.exec();
}
