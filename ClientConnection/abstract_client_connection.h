#ifndef ABSTARCTPERSOCLIENT_H
#define ABSTARCTPERSOCLIENT_H

#include "production_line_context_owner.h"
#include "types.h"

class AbstractClientConnection : public ProductionLineContextOwner {
  Q_OBJECT
 public:
  explicit AbstractClientConnection(const QString& name);
  virtual ~AbstractClientConnection();

 public slots:
  virtual void onInstanceThreadStarted(void) = 0;

 public:
  virtual int32_t id(void) const = 0;
  virtual void reset(void) = 0;

 private:
  Q_DISABLE_COPY_MOVE(AbstractClientConnection)

  void connectDependencies(void);

 signals:
  void logging(const QString& log);
  void disconnected();
  void shutdownProductionLine_signal(ReturnStatus& ret);
};

#endif  // ABSTARCTPERSOCLIENT_H
