#ifndef CONSOLERLOGBACKEND_H
#define CONSOLERLOGBACKEND_H

#include <QObject>
#include <QTextStream>

#include "log_backend.h"

class ConsolerLogBackend : public LogBackend {
  Q_OBJECT
 private:
  bool LogEnable;

 public:
  explicit ConsolerLogBackend(QObject* parent);
  ~ConsolerLogBackend();

  virtual void writeLogLine(const QString& str) override;
  virtual void clear() override;

 private:
  Q_DISABLE_COPY(ConsolerLogBackend);
  void loadSettings(void);
};

#endif  // CONSOLERLOGBACKEND_H
