#include <QCoreApplication>
#include <iostream>

#include "Management/server_manager.h"
#include "config.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

#ifdef PROJECT_VERSION
  std::cout << "Версия сервера: " << PROJECT_VERSION << std::endl;
#endif

  std::unique_ptr<ServerManager> manager(new ServerManager("ServerManager"));
  if (manager->init()) {
    QCoreApplication::exit(100);
  }

  return app.exec();
}
