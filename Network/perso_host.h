#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QSet>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>

#include "Management/transponder_release_system.h"
#include "perso_client_connection.h"

class PersoHost : public QTcpServer {
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
  OperatingState CurrentState;

  QSet<int32_t> FreeClientIds;
  QMap<int32_t, QThread*> ClientThreads;
  QMap<int32_t, PersoClientConnection*> Clients;

  TransponderReleaseSystem* Releaser;
  QThread* ReleaserThread;

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
  void applySettings_signal(void);
  void checkNewClientInstance(void);
  void operationFinished(ReturnStatus status);

  void startReleaser_signal(TransponderReleaseSystem::ReturnStatus* status);
  void stopReleaser_signal(void);
};

#endif  // PERSOSERVER_H
