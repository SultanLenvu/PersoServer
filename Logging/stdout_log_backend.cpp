#include <QString>
#include <QObject>
#include <QDebug>

#include "Logging/stdout_log_backend.h"

virtual void writeLogLine(const QString &str)
{
  qInfo() << str << "\n" << Qt::flush;
}

virtual void clear() {} /* No-op */
