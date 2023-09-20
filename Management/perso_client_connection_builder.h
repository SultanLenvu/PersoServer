#ifndef PERSOCLIENTCONNECTIONBUILDER_H
#define PERSOCLIENTCONNECTIONBUILDER_H

#include <QObject>
#include <QThread>

#include "perso_client_connection.h"

class PersoClientConnectionBuilder : public QObject {
  Q_OBJECT
 private:
  PersoClientConnection* Client;

 public:
  explicit PersoClientConnectionBuilder(void);
  PersoClientConnection* buildedObject() const;

 public slots:
  void build(uint32_t id, qintptr socketDescriptor);

 signals:
  void completed(void);
};

#endif // PERSOCLIENTCONNECTIONBUILDER_H
