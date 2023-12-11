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

#include "Log/console_log_backend.h"
#include "Log/file_log_backend.h"
#include "Log/log_backend.h"
#include "Log/udp_log_backend.h"

/* Глобальная система логгирования */
//==================================================================================

class LogSystem : public QObject {
  Q_OBJECT

 private:
  bool LogEnable;
  bool ExtendedLogEnable;

  QList<LogBackend*> Backends;
  UdpLogBackend* UdpLogger;
  FileLogBackend* FileLogger;
  ConsolerLogBackend* ConsoleLogger;

  QMutex mutex;

 public:
  ~LogSystem();
  static LogSystem* instance(void);

 public slots:
  void clear(void);
  void generate(const QString& log);

 private:
  LogSystem();
  explicit LogSystem(const QString& name);
  Q_DISABLE_COPY_MOVE(LogSystem)
  void loadSettings(void);
};

//==================================================================================

#endif  // LOGSYSTEM_H
