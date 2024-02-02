#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <algorithm>

#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QSet>
#include <QStack>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>
#include <QtPrintSupport/QPrinterInfo>

#include "abstract_client_connection.h"
#include "abstract_production_dispatcher.h"
#include "types.h"

class PersoServer : public QTcpServer {
  Q_OBJECT
 public:
  enum OperatingState {
    Idle,
    Work,
    Paused,
    Panic,
  };
  Q_ENUM(OperatingState);

 private:
  int32_t MaxNumberClientConnections;
  int32_t RestartPeriod;
  QHostAddress ListeningAddress;
  uint16_t ListeningPort;
  OperatingState CurrentState;

  QStack<int32_t> FreeClientIds;
  QHash<int32_t, std::shared_ptr<QThread>> ClientThreads;
  QHash<int32_t, std::shared_ptr<AbstractClientConnection>> Clients;

  std::unique_ptr<AbstractProductionDispatcher> Dispatcher;
  std::unique_ptr<QThread> DispatcherThread;

  std::unique_ptr<QTimer> RestartTimer;

 public:
  explicit PersoServer(const QString& name);
  ~PersoServer();

 public:
  bool start(void);
  void stop(void);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  PersoServer();
  Q_DISABLE_COPY_MOVE(PersoServer);
  void loadSettings(void);
  void sendLog(const QString& log);

  void processCriticalError(const QString& log);
  void deleteAllClientInstances(void);
  void deleteClientInstance(int32_t id);

  void createDispatcherInstance(void);
  void createClientIdentifiers(void);
  void createClientInstance(qintptr socketDescriptor);
  void createRestartTimer(void);

 private slots:
  void clientDisconnected_slot(void);
  void clientThreadDeleted_slot(void);

  void restartTimerTimeout_slot(void);

  void productionDispatcherErrorDetected(ReturnStatus status);

 signals:
  void logging(const QString& log);
  void startProductionDispatcher_signal(ReturnStatus&);
  void stopProductionDispatcher_signal(void);
};

#endif  // PERSOSERVER_H
