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
#include "Database/database_controller_interface.h"
#include "Database/postgres_controller.h"
#include "perso_host.h"
#include "user_settings.h"

class ServerManager : public QObject {
  Q_OBJECT

 private:
  PersoHost* Host;
  QThread* ServerThread;

  DatabaseBuffer* Buffer;

  QSettings* UserSettings;

 public:
  ServerManager(QObject* parent);
  ~ServerManager();

  DatabaseBuffer* buffer(void);

  void start(void);
  void stop(void);

  void showProductionLines(void);
  void showTransponders(void);
  void showOrders(void);
  void showIssuers(void);
  void showBoxes(void);
  void showPallets(void);
  void showCustomResponse(const QString& req);

 private:
  void createServerInstance(void);

 private slots:
  void proxyLogging(const QString& log);

  void serverThreadFinished(void);

 signals:
  void logging(const QString& log);
  void notifyUser(const QString& data);
  void notifyUserAboutError(const QString& data);

  // Сигналы для хоста
  void serverStart_signal(void);
  void serverStop_signal(void);

  void getProductionLines_signal(DatabaseBuffer* buffer);
  void getTransponders_signal(DatabaseBuffer* buffer);
  void getOrders_signal(DatabaseBuffer* buffer);
  void getIssuers_signal(DatabaseBuffer* buffer);
  void getBoxes_signal(DatabaseBuffer* buffer);
  void getPallets_signal(DatabaseBuffer* buffer);
  void getCustomResponse_signal(const QString& req, DatabaseBuffer* buffer);
};

//==================================================================================

#endif  // SERVER_MANAGER_H
