#ifndef PERSO_MANAGER_H
#define PERSO_MANAGER_H

#include <QElapsedTimer>
#include <QList>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <Qvector>

#include <QDebug>

#include "Database/database_controller_interface.h"
#include "Database/postgres_controller.h"
#include "user_settings.h"

/* Ядро управления сервером */
//==================================================================================

class PersoManager : public QObject {
  Q_OBJECT

 public:
  enum OperationStatus { Completed, Failed };

 private:
  OperationStatus LastOperationStatus;
  QThread* ExecutionThread;

  DatabaseControllerInterface* DatabaseController;
  QVector<QVector<QString>> DatabaseLastResponse;

  QTimer* DelayTimer;
  bool ReadyFlag;
  QElapsedTimer DurationTimer;
  bool EndlessOption;

 public:
  PersoManager(QObject* parent);
  ~PersoManager();

  // Функции для работы с контроллером базы данных
  void connectDatabase(void);
  void disconnectDatabase(void);
  void performCustomSqlRequest(const QString& req);

  void applySettings(UserSettings* settings) const;

 public slots:

 private:
  // Фабричные функции

 private slots:
  void delayEnd(void);
  void proxyLogging(const QString& log);

 private:
  void clearDatabaseLastResponse(void);
  void logDatabaseReponse(void);

 signals:
  void logging(const QString& log);
  void notifyUser(const QString& data);
  void notifyUserAboutError(const QString& data);
};

//==================================================================================

#endif  // PERSO_MANAGER_H
