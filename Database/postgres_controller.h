#ifndef POSTGRESCONTROLLER_H
#define POSTGRESCONTROLLER_H

#include <QHostAddress>
#include <QtSql>

#include "database_controller.h"

class PostgresController : public IDatabaseController {
  Q_OBJECT

 private:
  QString ConnectionName;
  QHostAddress HostAddress;
  uint32_t Port;
  QString DatabaseName;
  QString UserName;
  QString Password;

  QSqlQuery* CurrentRequest;

 public:
  explicit PostgresController(QObject* parent, const QString& connectionName);
  ~PostgresController();

 public:
  // IDatabaseController interface
  virtual void connect(void) override;
  virtual void disconnect(void) override;
  virtual bool isConnected(void) override;

  virtual void getObuByPAN(const QString& pan, DatabaseBuffer* buffer) override;
  virtual void getObuByUCID(const QString& ucid,
                            DatabaseBuffer* buffer) override;
  virtual void getObuBySerialNumber(const uint32_t serial,
                                    DatabaseBuffer* buffer) override;
  virtual void getObuListByContextMark(const QString& cm,
                                       DatabaseBuffer* buffer) override;
  virtual void getObuListBySerialNumber(const uint32_t serialBegin,
                                        const uint32_t serialEnd,
                                        DatabaseBuffer* buffer) override;
  virtual void getObuListByPAN(const uint32_t panBegin,
                               const uint32_t panEnd,
                               DatabaseBuffer* buffer) override;

  virtual void getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseBuffer* buffer) override;

  virtual void execCustomRequest(const QString& req,
                                 DatabaseBuffer* buffer) override;
  virtual void applySettings() override;

  bool addOrderToIssuer(const QString& issuerName);
  bool addOrder(const QString& issuerName);

 private:
  void loadSettings(void);
  void createDatabaseConnection(void);
  void convertResponseToBuffer(DatabaseBuffer* buffer);
};

#endif  // POSTGRESCONTROLLER_H
