#ifndef POSTGRESCONTROLLER_H
#define POSTGRESCONTROLLER_H

#include <QHostAddress>
#include <QtSql>

#include "database_controller_interface.h"

class PostgresController : public DatabaseControllerInterface {
  Q_OBJECT

 private:
  QHostAddress HostAddress;
  uint32_t Port;
  QString DatabaseName;
  QString UserName;
  QString Password;

  QSqlDatabase Postgres;
  QSqlQuery* CurrentRequest;

 public:
  explicit PostgresController(QObject* parent);

  virtual void connect(void) override;
  virtual void disconnect(void) override;

  virtual void getObuByPAN(const QString& pan,
                           QVector<QString>* result) override;
  virtual void getObuBySerialNumber(const uint32_t serial,
                                    QVector<QString>* result) override;
  virtual void getObuByUCID(const QString& ucid,
                            QVector<QString>* result) override;

  virtual void getObuListByContextMark(
      const QString& cm,
      QVector<QVector<QString>>* result) override;
  virtual void getObuListBySerialNumber(
      const uint32_t serialBegin,
      const uint32_t serialEnd,
      QVector<QVector<QString>>* result) override;
  virtual void getObuListByPAN(const uint32_t panBegin,
                               const uint32_t panEnd,
                               QVector<QVector<QString>>* result) override;

  virtual void execCustomRequest(const QString& req,
                                 QVector<QVector<QString>>* result) override;

  virtual void applySettings(UserSettings* settings) override;

 private:
  void createDatabase(void);
  void checkConnection(void);
};

#endif  // POSTGRESCONTROLLER_H
