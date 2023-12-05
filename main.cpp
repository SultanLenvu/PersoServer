#include <cstdlib>

#include <QCoreApplication>

#include "Management/server_manager.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  std::unique_ptr<ServerManager> manager(new ServerManager(nullptr));
  if (manager->init()) {
    exit(100);
  }

  return app.exec();
}
