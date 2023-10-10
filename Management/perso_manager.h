#ifndef PERSOMANAGER_H
#define PERSOMANAGER_H

#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QTextCodec>
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
  ~PersoManager();

  void processCommandArguments(const QStringList* args);

 private:
  Q_DISABLE_COPY(PersoManager);
  void loadSettings(void) const;
  bool checkSettings(void) const;
  void generateDefaultSettings(void) const;

  void createServerInstance(void);
  void createLoggerInstance(void);

 signals:
  void logging(const QString& log) const;
};

#endif // PERSOMANAGER_H
