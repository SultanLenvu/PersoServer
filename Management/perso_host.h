#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>

#include "Database/database_controller_interface.h"
#include "Database/postgres_controller.h"
#include "perso_client_connection.h"

class PersoHost : public QTcpServer {
  Q_OBJECT
 private:
  QList<QThread*> ClientThreads;
  QList<PersoClientConnection*> Clients;
  bool PauseIndicator;

  DatabaseControllerInterface* Database;

  QSettings* Settings;
  QMutex* Mutex;

 public:
  explicit PersoHost(QObject* parent, QSettings* settings);
  ~PersoHost();

 public slots:
  void start(void);
  void stop(void);

  void getProductionLines(DatabaseBuffer* buffer);
  void getTransponders(DatabaseBuffer* buffer);
  void getOrders(DatabaseBuffer* buffer);
  void getIssuers(DatabaseBuffer* buffer);
  void getBoxes(DatabaseBuffer* buffer);
  void getPallets(DatabaseBuffer* buffer);
  void getCustomResponse(const QString& req, DatabaseBuffer* buffer);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  void createClientInstance(qintptr socketDescriptor);

 private slots:
  void proxyLogging(const QString& log);

  void on_ClientDisconnected_slot(void);

  void on_ClientThreadDeleted_slot(void);
  void on_ClientConnectionDeleted_slot(void);

 signals:
  void logging(const QString& log);
  void checkNewClientInstance(void);
};

#endif  // PERSOSERVER_H
