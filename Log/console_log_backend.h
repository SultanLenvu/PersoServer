#ifndef CONSOLERLOGBACKEND_H
#define CONSOLERLOGBACKEND_H

#include <QObject>
#include <QTextStream>

#include "log_backend.h"

class ConsoleLogBackend : public LogBackend {
  Q_OBJECT
 private:
  bool LogEnable;

 public:
  explicit ConsoleLogBackend(const QString& name);
  ~ConsoleLogBackend();

  virtual void writeLogMessage(const QString& str) override;

 private:
  Q_DISABLE_COPY_MOVE(ConsoleLogBackend);
  void loadSettings(void);
};

#endif  // CONSOLERLOGBACKEND_H
