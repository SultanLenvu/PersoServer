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
  QList<LogBackend*> Backends;
  UdpLogBackend* WidgetLogger;
  FileLogBackend* FileLogger;
  ConsolerLogBackend* ConsoleLogger;

 public:
  ~LogSystem();
  static LogSystem* instance(void);

 public slots:
  void clear(void) const;
  void generate(const QString& log) const;

  void applySettings(void);

 private:
  LogSystem(QObject* parent);
  Q_DISABLE_COPY(LogSystem)
  void loadSettings(void);
};

//==================================================================================

#endif  // LOGSYSTEM_H
