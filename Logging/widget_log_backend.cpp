#include <QObject>
#include <QString>

#include "Logging/log_backend.h"
#include "Logging/widget_log_backend.h"
#include "GUI/gui.h"

WidgetLogBackend::WidgetLogBackend(QObject *parent, GUI *gui)
  : LogBackend(parent)
{
  connect(this, &WidgetLogBackend::requestDisplayLog,
      gui, &GUI::displayLog);
  connect(this, &WidgetLogBackend::requestClearDisplayLog,
      gui, &GUI::clearLogDisplay);
}

void WidgetLogBackend::writeLogLine(const QString &str)
{
  emit requestDisplayLog(str);
}

void WidgetLogBackend::clear()
{
  emit requestClearDisplayLog();
}
