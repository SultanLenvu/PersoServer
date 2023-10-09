#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <iostream>

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>

#include "Management/log_system.h"
#include "Management/transponder_release_system.h"
#include "Management/transponder_seed.h"
#include "perso_client_connection.h"

class PersoServer : public QTcpServer {
  Q_OBJECT
 public:
  enum OperatingState {
    Idle,
    Work,
    Paused,
  };
  enum ReturnStatus {
    Completed,
    Failed,
    ReleaserError,
  };

 private:
  int32_t MaxNumberClientConnections;
  QHostAddress ListeningAddress;
  uint32_t ListeningPort;
  OperatingState CurrentState;

  QSet<int32_t> FreeClientIds;
  QMap<int32_t, QThread*> ClientThreads;
  QMap<int32_t, PersoClientConnection*> Clients;

  TransponderReleaseSystem* Releaser;
  QThread* ReleaserThread;

  LogSystem* Logger;
  QThread* LoggerThread;

 public:
  explicit PersoServer(QObject* parent);
  ~PersoServer();

 public:
  bool start(void);
  bool stop(void);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  void loadSettings(void);
  void createReleaserInstance(void);
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

  void startReleaser_signal(TransponderReleaseSystem::ReturnStatus* status);
  void stopReleaser_signal(void);
};

#endif  // PERSOSERVER_H
