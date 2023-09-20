#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <QList>
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>

#include "Database/database_controller.h"
#include "Database/postgres_controller.h"
#include "perso_client_connection.h"

class PersoHost : public QTcpServer {
  Q_OBJECT
 private:
  int32_t MaxNumberClientConnections;

  QSet<int32_t> FreeClientIds;
  QMap<int32_t, QThread*> ClientThreads;
  QMap<int32_t, PersoClientConnection*> Clients;
  bool PauseIndicator;

  IDatabaseController* Database;

  QMutex Mutex;

 public:
  explicit PersoHost(QObject* parent);
  ~PersoHost();

 public slots:
  void start(void);
  void stop(void);
  void applySettings(void);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  void loadSettings(void);
  void createClientIdentifiers(void);
  void createClientInstance(qintptr socketDescriptor);

 private slots:
  void proxyLogging(const QString& log);

  void on_ClientDisconnected_slot(void);

  void on_ClientThreadDeleted_slot(void);
  void on_ClientConnectionDeleted_slot(void);

 signals:
  void logging(const QString& log);
  void checkNewClientInstance(void);
};

#endif  // PERSOSERVER_H
