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
  virtual void disconnect(TransactionResult result) override;

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
  void openTransaction(void) const;
  void closeTransaction(TransactionResult result) const;

  void loadSettings(void);
  void createDatabaseConnection(void);
  void convertResponseToBuffer(QSqlQuery& request,
                               DatabaseTableModel* buffer) const;
  void convertResponseToMap(QSqlQuery& request,
                            QMap<QString, QString>& record) const;
};

#endif  // POSTGRESCONTROLLER_H
