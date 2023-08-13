#ifndef PERSO_MANAGER_H
#define PERSO_MANAGER_H

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
#include "perso_server.h"
#include "user_settings.h"

/* Ядро управления сервером */
//==================================================================================

class PersoManager : public QObject {
  Q_OBJECT

 private:
  PersoServer* Server;
  QThread* ServerThread;

  DatabaseControllerInterface* DatabaseController;
  DatabaseBuffer* Buffer;

  QSettings* UserSettings;

 public:
  PersoManager(QObject* parent);
  ~PersoManager();

  DatabaseBuffer* buffer(void);

  // Функции для работы с сервером
  void startServer(void);
  void stopServer(void);

  // Функции для работы с контроллером базы данных
  void connectDatabase(void);
  void disconnectDatabase(void);
  void performCustomSqlRequest(const QString& req);

  void userSettings(void);

 private:
  // Фабричные функции
  void createServerInstance(void);

 private slots:
  void proxyLogging(const QString& log);

  void serverThreadFinished(void);

 signals:
  void logging(const QString& log);
  void notifyUser(const QString& data);
  void notifyUserAboutError(const QString& data);

  void serverStart_request(void);
  void serverStop_request(void);
};

//==================================================================================

#endif  // PERSO_MANAGER_H
