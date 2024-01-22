#ifndef LOGBACKEND_H
#define LOGBACKEND_H

#include <QObject>
#include <QSettings>

class LogBackend: public QObject {
 Q_OBJECT

 public:
  explicit LogBackend(const QString& name);
  ~LogBackend();

  virtual void writeLogLine(const QString& str) = 0;
  virtual void clear(void) = 0;

 private:
  Q_DISABLE_COPY_MOVE(LogBackend);
};

#endif /* LOGBACKEND_H */
