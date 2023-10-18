#include <QCoreApplication>

#include "Management/server_manager.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  QStringList cmdArgs = app.arguments();

  ServerManager manager(nullptr);
  manager.processCommandArguments(&cmdArgs);

  return app.exec();
}
