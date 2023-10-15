#ifndef LOGBACKEND_H
#define LOGBACKEND_H

#include <QObject>
#include <QSettings>

class LogBackend: public QObject {
 Q_OBJECT

 public:
  explicit LogBackend(QObject* parent);
  ~LogBackend();

  virtual void writeLogLine(const QString& str) = 0;
  virtual void clear(void) = 0;

 private:
  Q_DISABLE_COPY(LogBackend);
};

#endif /* LOGBACKEND_H */
