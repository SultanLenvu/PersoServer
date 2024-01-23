#ifndef ABSTARCTPERSOCLIENT_H
#define ABSTARCTPERSOCLIENT_H

#include <QObject>

class AbstractClientConnection : public QObject {
  Q_OBJECT
 public:
  explicit AbstractClientConnection(const QString& name);
  virtual ~AbstractClientConnection();

  virtual void onInstanceThreadStarted(void) = 0;
  virtual int32_t getId(void) const = 0;
  virtual void reset(void) = 0;

 private:
  AbstractClientConnection();
  Q_DISABLE_COPY_MOVE(AbstractClientConnection)

 signals:
  void logging(const QString& log);
  void disconnected(void);
};

#endif  // ABSTARCTPERSOCLIENT_H
