#ifndef WIDGETLOGBACKEND_H
#define WIDGETLOGBACKEND_H

#include <QObject>

#include "Logging/log_backend.h"
#include "GUI/gui.h"

class WidgetLogBackend: public LogBackend {
  Q_OBJECT
  public:
    WidgetLogBackend(QObject *parent, GUI *gui);

    virtual void writeLogLine(const QString &str) override;
    virtual void clear() override;

  signals:
    void requestDisplayLog(const QString& logData);
    void requestClearDisplayLog(void);
};

#endif /* WIDGETLOGBACKEND_H */
