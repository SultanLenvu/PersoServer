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
#include "Miscellaneous/thread_object_builder.h"
#include "order_creation_system.h"
#include "perso_host.h"
#include "user_settings.h"

class ServerManager : public QObject {
  Q_OBJECT

 public:
  enum OperationState { Ready, WaitingExecution, Failed, Completed };
  enum DatabaseTableIndex {
    RandomTable = 0,
    ProductionLineTable,
    TransponderTable,
    OrderTable,
    IssuerTable,
    BoxTable,
    PalletTable,
    CommercialKeyTable,
    TransportKeyTable,
    TableCounter
  };

 private:
  OperationState CurrentState;
  QString NotificarionText;

  PersoHost* Host;
  QThread* ServerThread;

  DatabaseTableModel* Buffer;

  OrderSystem* OrderCreator;
  OCSBuilder* OrderCreatorBuilder;
  QThread* OrderCreatorThread;

  QEventLoop* WaitingLoop;
  QTimer* ODTimer;
  QTimer* ODQTimer;
  QElapsedTimer* ODMeter;

 public:
  ServerManager(QObject* parent);
  ~ServerManager();

  DatabaseTableModel* buffer(void);
  void applySettings();

  void start(void);
  void stop(void);

  void showDatabaseTable(const QString& name);
  void clearDatabaseTable(const QString& name);
  void showCustomResponse(const QString& req);
  void createNewOrder(IssuerOrder* newOrder);

 private:
  void createHostInstance(void);
  void createOrderCreatorInstance(void);

  void createWaitingLoop(void);
  void createTimers(void);
  void setupODQTimer(uint32_t msecs);

  bool startOperationExecution(const QString& operationName);
  void endOperationExecution(const QString& operationName);

 private slots:
  void proxyLogging(const QString& log);

  void on_OrderCreatorBuilderCompleted_slot(void);
  void on_ServerThreadFinished_slot(void);
  void on_OrderCreatorThreadFinished_slot(void);

  void on_OrderCreatorFinished_slot(OrderSystem::ExecutionStatus status);
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

  // Сигналы для составителя заказов
  void getDatabaseTable_signal(const QString& tableName,
                               DatabaseTableModel* buffer);
  void clearDatabaseTable_signal(const QString& tableName);
  void getCustomResponse_signal(const QString& req, DatabaseTableModel* buffer);
  void createNewOrder_signal(IssuerOrder* order);
};

//==================================================================================

#endif  // SERVER_MANAGER_H
