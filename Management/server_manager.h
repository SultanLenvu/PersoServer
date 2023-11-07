#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QTextCodec>
#include <QThread>

#include "Log/log_system.h"
#include "Network/perso_server.h"

class ServerManager : public QObject {
  Q_OBJECT

 private:
  PersoServer* Server;

  LogSystem* Logger;
  QThread* LoggerThread;

 public:
  explicit ServerManager(QObject* parent);
  ~ServerManager();

  void processCommandArguments(const QStringList* args);
  bool checkSettings(void) const;
  void start();

 private:
  Q_DISABLE_COPY_MOVE(ServerManager);
  void loadSettings(void) const;
  void generateDefaultSettings(void) const;

  void createServerInstance(void);
  void createLoggerInstance(void);

  void registerMetaType(void);

 signals:
  void logging(const QString& log) const;
};

#endif  // SERVERMANAGER_H
