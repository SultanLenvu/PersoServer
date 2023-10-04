#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#include <QObject>
#include <QTime>
#include <QList>

#include "Logging/log_backend.h"

/* Глобальная система логгирования */
//==================================================================================

class LogSystem : public QObject {
  Q_OBJECT

 private:
  bool EnableIndicator;
  QList<LogBackend*> backends;

 public:
  LogSystem(QObject* parent);
  ~LogSystem();

  void clear(void);
  void setEnable(bool option);

  void addBackend(LogBackend *backend);
  void removeBackend(LogBackend *backend);

 public slots:
  void generate(const QString& log);

 signals:
  void requestDisplayLog(const QString& logData);
  void requestClearDisplayLog(void);
};

//==================================================================================

#endif  // LOGSYSTEM_H
