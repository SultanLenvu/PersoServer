#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QTcpServer>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "Database/database_buffer.h"
#include "Database/database_controller.h"
#include "Database/postgres_controller.h"
#include "perso_host.h"
#include "transponder_initializer.h"
#include "user_settings.h"

class ServerManager : public QObject {
  Q_OBJECT

 public:
  enum OperationState { Ready, WaitingExecution, Failed, Completed };

 private:
  OperationState CurrentState;
  QString NotificarionText;

  PersoHost* Host;
  QThread* ServerThread;

  TransponderInitializer* Initializer;
  QThread* InitializerThread;

  DatabaseBuffer* Buffer;

  QEventLoop* WaitingLoop;
  QTimer* ODTimer;
  QTimer* ODQTimer;
  QElapsedTimer* ODMeter;

 public:
  ServerManager(QObject* parent);
  ~ServerManager();

  DatabaseBuffer* buffer(void);
  void applySettings();

  void start(void);
  void stop(void);

  void showDatabaseTable(const QString& name);
  void showCustomResponse(const QString& req);

 private:
  void createHostInstance(void);
  void createInitializerInstance(void);

  void createWaitingLoop(void);
  void createTimers(void);
  void setupODQTimer(uint32_t msecs);

  bool startOperationExecution(const QString& operationName);
  void endOperationExecution(const QString& operationName);

 private slots:
  void proxyLogging(const QString& log);

  void on_ServerThreadFinished_slot(void);
  void on_InitializerThreadFinished_slot(void);

  void on_InitializerFinished_slot(
      TransponderInitializer::ExecutionStatus status);
  void on_ODTimerTimeout_slot(void);
  void on_ODQTimerTimeout_slot(void);

 signals:
  void logging(const QString& log);
  void notifyUser(const QString& data);
  void notifyUserAboutError(const QString& data);
  void operationPerfomingStarted(void);
  void operationStepPerfomed(void);
  void operationPerformingEnded(void);
  void waitingEnd(void);

  // Общий сигнал для применения настроек
  void applySettings_signal(void);

  // Сигналы для хоста
  void serverStart_signal(void);
  void serverStop_signal(void);

  // Сигналы для инициализатора
  void getDatabaseTable_signal(const QString& tableName,
                               DatabaseBuffer* buffer);
  void getCustomResponse_signal(const QString& req, DatabaseBuffer* buffer);
};

//==================================================================================

#endif  // SERVER_MANAGER_H
