#ifndef ABSTRACTSQLDATABASE_H
#define ABSTRACTSQLDATABASE_H

#include <QObject>

#include "sql_query_values.h"

class AbstractSqlDatabase : public QObject {
  Q_OBJECT

 public:
  explicit AbstractSqlDatabase(const QString& name);
  virtual ~AbstractSqlDatabase();

  virtual void applySettings() = 0;

  virtual bool connect(void) = 0;
  virtual void disconnect(void) = 0;
  virtual bool isConnected() = 0;

  virtual bool openTransaction(void) const = 0;
  virtual bool commitTransaction(void) const = 0;
  virtual bool rollbackTransaction(void) const = 0;

  virtual Qt::SortOrder getCurrentOrder(void) const = 0;
  virtual void setCurrentOrder(Qt::SortOrder order) = 0;

  virtual uint32_t getRecordMaxCount(void) const = 0;
  virtual void setRecordMaxCount(uint32_t count) = 0;

  virtual bool execCustomRequest(const QString& requestText,
                                 SqlQueryValues& records) const = 0;

  // Single table CRUD
  virtual bool createRecords(const QString& table,
                             const SqlQueryValues& records) const = 0;
  virtual bool readRecords(const QString& table,
                           SqlQueryValues& records) const = 0;
  virtual bool readRecords(const QString& table,
                           const QString& conditions,
                           SqlQueryValues& records) const = 0;
  virtual bool readLastRecord(const QString& table,
                              SqlQueryValues& record) const = 0;
  virtual bool updateRecords(const QString& table,
                             const SqlQueryValues& newValues) const = 0;
  virtual bool updateRecords(const QString& table,
                             const QString& condition,
                             const SqlQueryValues& newValues) const = 0;
  virtual bool deleteRecords(const QString& table,
                             const QString& condition) const = 0;
  virtual bool clearTable(const QString& table) const = 0;

  // Multi table CRUD
  virtual bool readMergedRecords(const QStringList& tables,
                                 const QString& conditions,
                                 SqlQueryValues& records) const = 0;
  virtual bool updateMergedRecords(const QStringList& tables,
                                   const QString& conditions,
                                   const SqlQueryValues& newValues) const = 0;
  virtual bool deleteMergedRecords(const QStringList& tables,
                                   const QString& conditions) const = 0;

  // Aggregation
  virtual bool getRecordCount(const QString& table, uint32_t& count) const = 0;

 private:
  AbstractSqlDatabase();
  Q_DISABLE_COPY_MOVE(AbstractSqlDatabase)

 signals:
  void logging(const QString& log);
  void disconnected(void);
};

#endif  // ABSTRACTSQLDATABASE_H
