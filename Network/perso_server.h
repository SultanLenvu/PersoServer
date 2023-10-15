#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <iostream>

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QStack>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>

#include "Log/log_system.h"
#include "Management/transponder_release_system.h"
#include "Management/transponder_seed.h"
#include "perso_client.h"

class PersoServer : public QTcpServer {
  Q_OBJECT
 public:
  enum OperatingState {
    Idle,
    Work,
    Paused,
  };
  Q_ENUM(OperatingState);
  enum ReturnStatus {
    Completed,
    Failed,
    ReleaserError,
  };
  Q_ENUM(ReturnStatus);

 private:
  bool LogEnable;

  int32_t MaxNumberClientConnections;
  QHostAddress ListeningAddress;
  uint32_t ListeningPort;
  OperatingState CurrentState;

  QStack<int32_t> FreeClientIds;
  QMap<int32_t, QThread*> ClientThreads;
  QMap<int32_t, PersoClient*> Clients;

  TransponderReleaseSystem* Releaser;
  QThread* ReleaserThread;

 public:
  explicit PersoServer(QObject* parent);
  ~PersoServer();

 public:
  bool start(void);
  void stop(void);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  Q_DISABLE_COPY(PersoServer);
  void loadSettings(void);
  void sendLog(const QString& log) const;
  void createReleaserInstance(void);
  void createClientIdentifiers(void);
  void createClientInstance(qintptr socketDescriptor);

 private slots:
  void on_ClientDisconnected_slot(void);

  void on_ClientThreadDeleted_slot(void);
  void on_ClientConnectionDeleted_slot(void);

 signals:
  void logging(const QString& log) const;
  void checkNewClientInstance(void);

  void startReleaser_signal(TransponderReleaseSystem::ReturnStatus* status);
  void stopReleaser_signal(void);
};

#endif  // PERSOSERVER_H
