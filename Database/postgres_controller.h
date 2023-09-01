#ifndef POSTGRESCONTROLLER_H
#define POSTGRESCONTROLLER_H

#include <QHostAddress>
#include <QMap>
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

 public:
  explicit PostgresController(QObject* parent, const QString& connectionName);
  ~PostgresController();

 public:
  // IDatabaseController interface
  virtual bool connect(void) override;
  virtual void disconnect(bool resultOption) override;
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

  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) override;
  virtual void applySettings() override;

  bool clearTable(const QString& tableName) const;

  bool addRecord(const QString& tableName,
                 QMap<QString, QString>& record) const;

  bool getRecordById(const QString& tableName,
                     QMap<QString, QString>& record) const;
  bool getRecordByPart(const QString& tableName,
                       QMap<QString, QString>& record) const;
  bool getLastRecord(const QString& tableName,
                     QMap<QString, QString>& record) const;

  bool getMergedRecordById(const QStringList& tables,
                           const QStringList& foreignKeys,
                           QMap<QString, QString>& record) const;
  bool getMergedRecordByPart(const QStringList& tables,
                             const QStringList& foreignKeys,
                             QMap<QString, QString>& record) const;

  bool updateRecord(const QString& tableName,
                    QMap<QString, QString>& record) const;
  bool removeRecordById(const QString& tableName, const uint32_t id) const;
  bool removeLastRecord(const QString& tableName) const;
  bool removeLastRecordWithCondition(const QString& tableName,
                                     const QString& condition) const;

 private:
  bool openTransaction(void) const;
  bool closeTransaction(void) const;
  bool abortTransaction(void) const;

  void loadSettings(void);
  void createDatabaseConnection(void);
  void convertResponseToBuffer(QSqlQuery& request,
                               DatabaseTableModel* buffer) const;
  void convertResponseToMap(QSqlQuery& request,
                            QMap<QString, QString>& record) const;
};

#endif  // POSTGRESCONTROLLER_H
