#include "Network/perso_server.h"

#include <QApplication>
#include <QStringList>

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  QStringList cmdArgs = app.arguments();

  PersoServer server(nullptr);
  return app.exec();
}
