#include <QString>
#include <QObject>
#include <QDebug>

#include "Logging/stdout_log_backend.h"

void StdoutLogBackend::writeLogLine(const QString &str)
{
  qInfo() << str << "\n" << Qt::flush;
}

void StdoutLogBackend::clear() {} /* No-op */
