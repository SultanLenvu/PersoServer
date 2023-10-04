#ifndef SYSLOGLOGBACKEND_H
#define SYSLOGLOGBACKEND_H

#include <QObject>
#include <QString>

#include "Logging/log_backend.h"

class SyslogLogBackend: public LogBackend {
  Q_OBJECT

  public:
    SyslogLogBackend(QObject *parent);
    ~SyslogLogBackend();

    virtual void writeLogLine(const QString &str);
    virtual void clear();
};

#endif /* SYSLOGLOGBACKEND_H */
