#ifndef LOGBACKEND_H
#define LOGBACKEND_H

#include <QObject>
#include <QSettings>

class LogBackend: public QObject {
 Q_OBJECT

 public:
  explicit LogBackend(const QString& name);
  ~LogBackend();

  virtual void writeLogMessage(const QString& str) = 0;

 private:
  LogBackend();
  Q_DISABLE_COPY_MOVE(LogBackend);
};

#endif /* LOGBACKEND_H */
