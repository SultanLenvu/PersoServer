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
  virtual bool disconnect(void) override;

  virtual bool openTransaction(void) const override;
  virtual bool closeTransaction(TransactionResult result) const override;

  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) const override;
  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) const override;
  virtual void applySettings() override;

  virtual bool clearTable(const QString& tableName) const override;

  virtual bool addRecord(const QString& tableName,
                         QMap<QString, QString>& record) const override;

  virtual bool getRecordById(const QString& tableName,
                             QMap<QString, QString>& record) const override;
  virtual bool getRecordByPart(const QString& tableName,
                               QMap<QString, QString>& record) const override;
  virtual bool getLastRecord(const QString& tableName,
                             QMap<QString, QString>& record) const override;

  virtual bool getMergedRecordById(
      const QStringList& tables,
      const QStringList& foreignKeys,
      QMap<QString, QString>& record) const override;
  virtual bool getMergedRecordByPart(
      const QStringList& tables,
      const QStringList& foreignKeys,
      QMap<QString, QString>& record) const override;

  virtual bool updateRecord(const QString& tableName,
                            QMap<QString, QString>& record) const override;
  virtual bool removeRecordById(const QString& tableName,
                                const uint32_t id) const override;
  virtual bool removeLastRecord(const QString& tableName) const override;
  virtual bool removeLastRecordWithCondition(
      const QString& tableName,
      const QString& condition) const override;

 private:
  void loadSettings(void);
  void createDatabaseConnection(void);
  void convertResponseToBuffer(QSqlQuery& request,
                               DatabaseTableModel* buffer) const;
  void convertResponseToMap(QSqlQuery& request,
                            QMap<QString, QString>& record) const;
};

#endif  // POSTGRESCONTROLLER_H
