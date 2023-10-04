#ifndef LOGBACKEND_H
#define LOGBACKEND_H

#include <QObject>

class LogBackend: public QObject {
 Q_OBJECT

 public:
  LogBackend(QObject* parent);
  ~LogBackend();

  virtual void writeLogLine(const QString &str) = 0;
  virtual void clear() = 0;
};

#endif /* LOGBACKEND_H */
