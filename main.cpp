#include "GUI/main_window_kernel.h"

#include <QApplication>
#include <QScopedPointer>
#include <QTextCodec>

bool noGui(int argc, char* argv[]) {
  for (char **arg = argv; *arg; arg++)
    if (!qstrcmp(*arg, "--no-gui"))
      return true;
  return false;
}

int main(int argc, char* argv[]) {
  // Установка кодировки UTF-8 для консоли вывода
#elif _WIN32
  QTextCodec::setCodecForLocale(QTextCodec::codecForName("CP1251"));
#endif /* _WIN32 */

  if (noGui(argc, argv)) {
    QCoreApplication app(argc, argv);
    return app.exec();
  }
  else {
    QApplication app(argc, argv);
    MainWindowKernel w;
    w.show();
    return app.exec()
  }
}
