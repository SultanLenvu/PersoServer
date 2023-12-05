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

#include "ClientConnection/abstract_client_connection.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

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
  bool LogEnable;

  int32_t MaxNumberClientConnections;
  int32_t RestartPeriod;
  QHostAddress ListeningAddress;
  uint32_t ListeningPort;
  OperatingState CurrentState;

  QStack<int32_t> FreeClientIds;
  QHash<int32_t, std::unique_ptr<QThread>> ClientThreads;
  QHash<int32_t, std::unique_ptr<AbstractClientConnection>> Clients;

  std::unique_ptr<AbstractProductionDispatcher> ProductionDispatcher;
  std::unique_ptr<QThread> ProductionDispatcherThread;

  std::unique_ptr<QTimer> RestartTimer;

 public:
  explicit PersoServer();
  ~PersoServer();

 public:
  bool start(void);
  void stop(void);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  Q_DISABLE_COPY_MOVE(PersoServer);
  void loadSettings(void);
  void sendLog(const QString& log) const;

  void processCriticalError(const QString& log);
  bool checkConfiguration(void);

  void createProductionDispatcherInstance(void);
  void createClientIdentifiers(void);
  void createClientInstance(qintptr socketDescriptor);
  void createRestartTimer(void);

 private slots:
  void clientDisconnected_slot(void);
  void clientThreadDeleted_slot(void);

  void restartTimerTimeout_slot(void);

  void productionDispatcherErrorDetected(ReturnStatus& status);

 signals:
  void logging(const QString& log);

  void startProductionDispatcher_signal(ReturnStatus&);
  void stopProductionDispatcher_signal(void);
};

#endif  // PERSOSERVER_H
