#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#include <QHostAddress>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QThread>
#include <QTime>
#include <QUdpSocket>

#include "log_backend.h"

/* Глобальная система логгирования */
//==================================================================================

class LogSystem : public QObject {
  Q_OBJECT

 private:
  bool LogEnable;
  bool ExtendedLogEnable;

  std::vector<std::shared_ptr<LogBackend>> Backends;

 public:
  explicit LogSystem(const QString& name);
  ~LogSystem();

 public slots:
  void clear(void);
  void generate(const QString& log);

 private:
  LogSystem();
  Q_DISABLE_COPY_MOVE(LogSystem)
  void loadSettings(void);
};

//==================================================================================

#endif  // LOGSYSTEM_H
