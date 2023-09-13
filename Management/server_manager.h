#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <QElapsedTimer>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSettings>
#include <QTcpServer>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "Database/database_controller.h"
#include "Database/database_table_model.h"
#include "Database/postgres_controller.h"
#include "Miscellaneous/thread_object_builder.h"
#include "administration_system.h"
#include "administration_system_builder.h"
#include "perso_host.h"
#include "transponder_data_model.h"
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

  AdministrationSystem* Administrator;
  AdministrationSystemBuilder* AdministratorBuilder;
  QThread* AdministratorThread;

  QEventLoop* WaitingLoop;
  QTimer* ODTimer;
  QTimer* ODQTimer;
  QElapsedTimer* ODMeter;

 public:
  ServerManager(QObject* parent);
  ~ServerManager();

  void applySettings();

  void start(void);
  void stop(void);

  void showDatabaseTable(const QString& name, DatabaseTableModel* model);
  void clearDatabaseTable(const QString& name, DatabaseTableModel* model);
  void initIssuers(DatabaseTableModel* model);
  void initTransportMasterKeys(const QString& issuerId,
                               DatabaseTableModel* model);

  void performCustomRequest(const QString& req, DatabaseTableModel* model);

  void createNewOrder(const QMap<QString, QString>* orderParameters,
                      DatabaseTableModel* model);
  void startOrderAssemblingManually(const QString& orderId,
                                    DatabaseTableModel* model);
  void stopOrderAssemblingManually(const QString& orderId,
                                   DatabaseTableModel* model);
  void deleteLastOrder(DatabaseTableModel* model);
  void showOrderTable(DatabaseTableModel* model);

  void createNewProductionLine(
      const QMap<QString, QString>* productionLineParameters,
      DatabaseTableModel* model);
  void allocateInactiveProductionLinesManually(const QString& orderId,
                                               DatabaseTableModel* model);
  void shutdownAllProductionLinesManually(DatabaseTableModel* model);
  void deleteLastProductionLine(DatabaseTableModel* model);
  void showProductionLineTable(DatabaseTableModel* model);
  void linkProductionLineWithBoxManually(
      const QMap<QString, QString>* linkParameters,
      DatabaseTableModel* model);

  void releaseTransponderManually(TransponderDataModel* model);
  void confirmReleaseTransponderManually(TransponderDataModel* model);
  void rereleaseTransponderManually(TransponderDataModel* model);
  void confirmRereleaseTransponderManually(TransponderDataModel* model);
  void searchTransponderManually(TransponderDataModel* model);
  void refundTransponderManually(TransponderDataModel* model);

 private:
  void createHostInstance(void);
  void createAdministratorInstance(void);

  void createWaitingLoop(void);
  void createTimers(void);
  void setupODQTimer(uint32_t msecs);

  bool startOperationExecution(const QString& operationName);
  void endOperationExecution(const QString& operationName);

 private slots:
  void proxyLogging(const QString& log);

  void on_AdministratorBuilderCompleted_slot(void);
  void on_ServerThreadFinished_slot(void);
  void on_AdministratorThreadFinished_slot(void);

  void on_AdministratorFinished_slot(
      AdministrationSystem::ExecutionStatus status);
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
  void startServer_signal(void);
  void stopServer_signal(void);

  // Сигналы для системы администрирования
  void getDatabaseTable_signal(const QString& tableName,
                               DatabaseTableModel* model);
  void clearDatabaseTable_signal(const QString& tableName);
  void getCustomResponse_signal(const QString& req, DatabaseTableModel* model);
  void initIssuerTable_signal(void);
  void initTransportMasterKeysTable_signal(const QString& issuerId);

  void createNewOrder_signal(const QMap<QString, QString>* orderParameters);
  void startOrderAssembling_signal(const QString& orderId);
  void stopOrderAssembling_signal(const QString& orderId);
  void deleteLastOrder_signal(void);

  void createNewProductionLine_signal(
      const QMap<QString, QString>* productionLineParameters);
  void allocateInactiveProductionLines_signal(const QString& orderId);
  void shutdownAllProductionLines_signal(void);
  void removeLastProductionLine_signal(void);
  void linkProductionLineWithBox_signal(
      const QMap<QString, QString>* linkParameters);

  // Сигналы для системы выпуска транспондеров
  void releaseTransponder_signal(TransponderDataModel* seed);
  void confirmReleaseTransponder_signal(TransponderDataModel* seed);
  void rereleaseTransponder_signal(TransponderDataModel* seed);
  void confirmRereleaseTransponder_signal(TransponderDataModel* seed);
  void searchTransponder_signal(TransponderDataModel* seed);
};

//==================================================================================

#endif  // SERVER_MANAGER_H
