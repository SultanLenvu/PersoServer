#include <cstdlib>

#include <QCoreApplication>

#include "Management/server_manager.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  ServerManager manager(nullptr);
  if (manager.init()) {
    exit(100);
  }

  return app.exec();
}
