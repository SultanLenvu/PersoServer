#ifndef PostgreSqlDatabase_H
#define PostgreSqlDatabase_H

#include <QHostAddress>
#include <QVector>
#include <QtSql>

#include "abstract_sql_database.h"
#include "postgre_sql_table.h"

class PostgreSqlDatabase : public AbstractSqlDatabase {
  Q_OBJECT

 private:
  QString ConnectionName;

  QString CurrentOrder;
  uint32_t RecordMaxCount;

  QString DatabaseName;
  QHostAddress HostAddress;
  uint32_t HostPort;
  QString UserName;
  QString UserPassword;

  QHash<QString, std::shared_ptr<PostgreSqlTable>> Tables;

 public:
  explicit PostgreSqlDatabase(const QString& name);
  ~PostgreSqlDatabase();

  // AbstractSqlDatabase interface
  virtual void applySettings() override;

  virtual bool connect() override;
  virtual void disconnect() override;
  virtual bool isConnected() override;

  virtual bool openTransaction() const override;
  virtual bool commitTransaction() const override;
  virtual bool rollbackTransaction() const override;

  virtual Qt::SortOrder getCurrentOrder() const override;
  virtual void setCurrentOrder(Qt::SortOrder order) override;

  virtual uint32_t getRecordMaxCount(void) const override;
  virtual void setRecordMaxCount(uint32_t count) override;

  virtual bool execCustomRequest(const QString& requestText,
                                 SqlQueryValues& records) const override;

  // Single table CRUD
  virtual bool createRecords(const QString& table,
                             const SqlQueryValues& records) const override;
  virtual bool readRecords(const QString& table,
                           SqlQueryValues& records) const override;
  virtual bool readRecords(const QString& table,
                           const QString& conditions,
                           SqlQueryValues& records) const override;
  virtual bool readLastRecord(const QString& table,
                              SqlQueryValues& record) const override;
  virtual bool updateRecords(const QString& table,
                             const SqlQueryValues& newValues) const override;
  virtual bool updateRecords(const QString& table,
                             const QString& conditions,
                             const SqlQueryValues& newValues) const override;
  virtual bool deleteRecords(const QString& table,
                             const QString& conditions) const override;
  virtual bool clearTable(const QString& table) const override;

  // Multi table CRUD
  virtual bool readMergedRecords(const QStringList& tables,
                                 const QString& conditions,
                                 SqlQueryValues& records) const override;
  virtual bool updateMergedRecords(
      const QStringList& tables,
      const QString& conditions,
      const SqlQueryValues& newValues) const override;
  virtual bool deleteMergedRecords(const QStringList& tables,
                                   const QString& conditions) const override;

  // Aggregation
  virtual bool getRecordCount(const QString& table,
                              uint32_t& count) const override;

 private:
  Q_DISABLE_COPY_MOVE(PostgreSqlDatabase)
  void sendLog(const QString& log) const;
  void loadSettings(void);

  void createDatabaseConnection(void);
  bool init(void);
  bool createTable(const QString& name);

  bool checkTableNames(const QStringList& names) const;
};

#endif  // PostgreSqlDatabase_H
