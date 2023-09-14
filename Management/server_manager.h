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
#include "transponder_seed_model.h"
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

  void performCustomRequest(const QString& req, DatabaseTableModel* model);

  void createNewOrder(const QMap<QString, QString>* orderParameterseters,
                      DatabaseTableModel* model);
  void startOrderAssemblingManually(const QString& orderId,
                                    DatabaseTableModel* model);
  void stopOrderAssemblingManually(const QString& orderId,
                                   DatabaseTableModel* model);
  void deleteLastOrder(DatabaseTableModel* model);
  void showOrderTable(DatabaseTableModel* model);

  void createNewProductionLine(
      const QMap<QString, QString>* productionLineParameterseters,
      DatabaseTableModel* model);
  void allocateInactiveProductionLinesManually(const QString& orderId,
                                               DatabaseTableModel* model);
  void shutdownAllProductionLinesManually(DatabaseTableModel* model);
  void deleteLastProductionLine(DatabaseTableModel* model);
  void showProductionLineTable(DatabaseTableModel* model);
  void linkProductionLineWithBoxManually(
      const QMap<QString, QString>* linkParameterseters,
      DatabaseTableModel* model);

  void releaseTransponderManually(
      const QMap<QString, QString>* releaseParameters,
      TransponderSeedModel* model);
  void confirmReleaseTransponderManually(
      const QMap<QString, QString>* confirmParameters);
  void rereleaseTransponderManually(
      const QMap<QString, QString>* rereleaseParameters,
      TransponderSeedModel* model);
  void confirmRereleaseTransponderManually(
      const QMap<QString, QString>* confirmParameters);
  void searchTransponderManually(const QMap<QString, QString>* searchParameters,
                                 TransponderSeedModel* model);
  void refundTransponderManually(const QMap<QString, QString>* refundParameters,
                                 TransponderSeedModel* model);

  void initIssuers(DatabaseTableModel* model);
  void initTransportMasterKeys(DatabaseTableModel* model);
  void linkIssuerWithMasterKeys(DatabaseTableModel* model,
                                const QMap<QString, QString>* Parameterseters);

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

  void createNewOrder_signal(
      const QMap<QString, QString>* orderParameterseters);
  void startOrderAssembling_signal(const QString& orderId);
  void stopOrderAssembling_signal(const QString& orderId);
  void deleteLastOrder_signal(void);

  void createNewProductionLine_signal(
      const QMap<QString, QString>* productionLineParameterseters);
  void allocateInactiveProductionLines_signal(const QString& orderId);
  void shutdownAllProductionLines_signal(void);
  void removeLastProductionLine_signal(void);
  void linkProductionLineWithBox_signal(
      const QMap<QString, QString>* linkParameterseters);

  void releaseTransponder_signal(
      const QMap<QString, QString>* releaseParameters,
      TransponderSeedModel* seed);
  void confirmReleaseTransponder_signal(
      const QMap<QString, QString>* confirmParameters);
  void rereleaseTransponder_signal(
      const QMap<QString, QString>* rereleaseParameters,
      TransponderSeedModel* seed);
  void confirmRereleaseTransponder_signal(
      const QMap<QString, QString>* confirmParameters);
  void searchTransponder_signal(const QMap<QString, QString>* searchParameters,
                                TransponderSeedModel* seed);
  void refundTransponder_signal(const QMap<QString, QString>* refundParameters,
                                TransponderSeedModel* seed);

  void initIssuerTable_signal(void);
  void initTransportMasterKeysTable_signal(void);
  void linkIssuerWithMasterKeys_signal(
      const QMap<QString, QString>* linkParameterseters);
};

//==================================================================================

#endif  // SERVER_MANAGER_H
