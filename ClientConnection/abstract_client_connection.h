#ifndef ABSTARCTPERSOCLIENT_H
#define ABSTARCTPERSOCLIENT_H

#include <QObject>

class AbstractClientConnection : public QObject {
  Q_OBJECT
 public:
  explicit AbstractClientConnection(const QString& name);
  virtual ~AbstractClientConnection();

  virtual size_t getId(void) const = 0;

 private:
  AbstractClientConnection();
  Q_DISABLE_COPY_MOVE(AbstractClientConnection)

 signals:
  void logging(const QString& log);
  void disconnected(void);
};

#endif  // ABSTARCTPERSOCLIENT_H
