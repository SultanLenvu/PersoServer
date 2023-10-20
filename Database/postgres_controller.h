#ifndef POSTGRESCONTROLLER_H
#define POSTGRESCONTROLLER_H

#include <QHostAddress>
#include <QHash>
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
  virtual void disconnect(void) override;
  virtual bool isConnected(void) override;

  virtual bool openTransaction(void) const override;
  virtual bool closeTransaction(void) const override;
  virtual bool abortTransaction(void) const override;

  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) const override;
  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) const override;
  virtual bool clearTable(const QString& tableName) const override;

  virtual bool addRecord(const QString& tableName,
                         QHash<QString, QString>& record) const override;

  virtual bool getRecordById(const QString& tableName,
                             QHash<QString, QString>& record) const override;
  virtual bool getRecordByPart(const QString& tableName,
                               QHash<QString, QString>& record,
                               bool order = true) const override;
  virtual bool getLastRecord(const QString& tableName,
                             QHash<QString, QString>& record) const override;

  virtual bool getMergedRecordById(
      const QStringList& tables,
      const QStringList& foreignKeys,
      QHash<QString, QString>& record) const override;
  virtual bool getMergedRecordByPart(
      const QStringList& tables,
      const QStringList& foreignKeys,
      QHash<QString, QString>& record) const override;

  virtual bool updateRecordById(const QString& tableName,
                            QHash<QString, QString>& record) const override;
  bool updateAllRecordsByPart(const QString& tableName,
                              QHash<QString, QString>& conditions,
                              QHash<QString, QString>& newValues) const;
  virtual bool removeRecordById(const QString& tableName,
                                const uint32_t id) const override;
  virtual bool removeLastRecordById(const QString& tableName) const override;
  virtual bool removeLastRecordByCondition(
      const QString& tableName,
      QHash<QString, QString>& condition) const override;

  virtual void applySettings() override;

 private:
  Q_DISABLE_COPY(PostgresController);
  void loadSettings(void);
  void sendLog(const QString& log) const;
  void createDatabaseConnection(void);
  void convertResponseToBuffer(QSqlQuery& request,
                               DatabaseTableModel* buffer) const;
  void convertResponseToMap(QSqlQuery& request,
                            QHash<QString, QString>& record) const;
};

#endif  // POSTGRESCONTROLLER_H
