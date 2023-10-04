#include <QObject>
#include <QString>

#include "Logging/log_backend.h"

class StdoutLogBackend: public LogBackend {
  Q_OBJECT

  public:
    virtual void writeLogLine(const QString &str) override;
    virtual void clear() override;
};
