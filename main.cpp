#include "GUI/main_window_kernel.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char* argv[]) {
  // Установка кодировки UTF-8 для консоли вывода
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP1251"));

  QApplication a(argc, argv);
  MainWindowKernel w;
  w.show();
  return a.exec();
}
