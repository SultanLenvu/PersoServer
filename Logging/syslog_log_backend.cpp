#include <syslog.h>

#include <QObject>
#include <QString>
#include <QByteArray>

#include "General/definitions.h"
#include "Logging/log_backend.h"
#include "Logging/syslog_log_backend.h"

SyslogLogBackend::SyslogLogBackend(QObject *parent): LogBackend(parent)
{
  openlog(PROGRAM_NAME, LOG_ODELAY, LOG_USER);
}

SyslogLogBackend::~SyslogLogBackend()
{
  closelog();
}

void SyslogLogBackend::writeLogLine(const QString &str)
{
  QByteArray data = str.toUtf8();
  syslog(LOG_INFO, data.data());
}
void clear() {} /* No-op */
