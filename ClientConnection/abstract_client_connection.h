#ifndef ABSTARCTPERSOCLIENT_H
#define ABSTARCTPERSOCLIENT_H

#include <QObject>

#include "General/types.h"

class AbstractClientConnection : public QObject {
  Q_OBJECT
 public:
  explicit AbstractClientConnection(const QString& name);
  virtual ~AbstractClientConnection();

  virtual size_t getId(void) const = 0;
  virtual bool isAuthorised(void) const = 0;
  virtual const QString& getLogin(void) const = 0;
  virtual const QString& getPassword(void) const = 0;

 private:
  AbstractClientConnection();
  Q_DISABLE_COPY_MOVE(AbstractClientConnection)

 signals:
  void logging(const QString& log);
  void disconnected(void);
  void logOut_signal(const StringDictionary& param, ReturnStatus& status);
};

#endif  // ABSTARCTPERSOCLIENT_H
