#ifndef POSTGRESCONTROLLER_H
#define POSTGRESCONTROLLER_H

#include <QHostAddress>
#include <QMap>
#include <QtSql>

#include "Management/issuer_order.h"
#include "database_controller.h"
#include "table_record.h"

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

  virtual void getObuByPAN(const QString& pan,
                           DatabaseTableModel* buffer) override;
  virtual void getObuByUCID(const QString& ucid,
                            DatabaseTableModel* buffer) override;
  virtual void getObuBySerialNumber(const uint32_t serial,
                                    DatabaseTableModel* buffer) override;
  virtual void getObuListByContextMark(const QString& cm,
                                       DatabaseTableModel* buffer) override;
  virtual void getObuListBySerialNumber(const uint32_t serialBegin,
                                        const uint32_t serialEnd,
                                        DatabaseTableModel* buffer) override;
  virtual void getObuListByPAN(const uint32_t panBegin,
                               const uint32_t panEnd,
                               DatabaseTableModel* buffer) override;

  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) override;

  virtual void execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) override;
  virtual void applySettings() override;

  bool clearTable(const QString& tableName) const;
  int32_t getIdByAttribute(const QString& tableName,
                           QPair<QString, QString>& attribute) const;
  int32_t getIdByCondition(const QString& tableName,
                           const QString& condition,
                           bool minMaxOption);
  bool increaseAttributeValue(const QString& tableName,
                              const QString& attributeName,
                              const QString& id,
                              uint32_t value);
  bool addTableRecord(const QString& tableName,
                      QMap<QString, QString>& record) const;

 private:
  void loadSettings(void);
  void createDatabaseConnection(void);
  void convertResponseToBuffer(DatabaseTableModel* buffer);
};

#endif  // POSTGRESCONTROLLER_H
