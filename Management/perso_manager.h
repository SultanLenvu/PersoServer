#ifndef PERSOMANAGER_H
#define PERSOMANAGER_H

#include <QObject>
#include <QStringList>
#include <QThread>

#include "Network/perso_server.h"
#include "log_system.h"

class PersoManager : public QObject
{
  Q_OBJECT

 private:
  PersoServer* Server;

  LogSystem* Logger;
  QThread* LoggerThread;

 public:
  explicit PersoManager(QObject* parent);
  void processCommandArguments(const QStringList* args);

 private:
  void loadSettings(void);

  void createServerInstance(void);
  void createLoggerInstance(void);
};

#endif // PERSOMANAGER_H
