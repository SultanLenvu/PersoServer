#include <iostream>

#include <QApplication>
#include <QStringList>
#include <QTextStream>

#include "Management/perso_manager.h"

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  QStringList cmdArgs = app.arguments();

  PersoManager manager(nullptr);
  manager.processCommandArguments(&cmdArgs);

  return app.exec();
}
