#ifndef SQLTABLE_H
#define SQLTABLE_H

#include <QHash>
#include <QScopedPointer>
#include <QString>
#include <QtSql>

#include "abstract_sql_table.h"

class PostgreSqlTable : public AbstractSqlTable {
 private:
  QString ConnectionName;

  QString PrimaryKey;
  QVector<QString> Columns;
  QHash<QString, QString> Relations;

  uint32_t RecordMaxCount;
  QString CurrentOrder;

 public:
  explicit PostgreSqlTable(const QString& name, const QString& connectionName);
  ~PostgreSqlTable();

  // Accessors
  QString getPrimaryKey() const;
  void setPrimaryKey(const QString& newPrimaryKey);

  uint32_t getRecordMaxCount() const;
  void setRecordMaxCount(uint32_t newRecordMaxCount);

  Qt::SortOrder getCurrentOrder() const;
  void setCurrentOrder(Qt::SortOrder order);

  const QVector<QString>* columns() const;
  const QHash<QString, QString>* relations() const;

  // AbstractSqlTable interface
 public:
  virtual bool init() override;
  virtual void applySettings() override;

  // CRUD
  virtual bool createRecords(const SqlQueryValues& response) const override;
  virtual bool readRecords(SqlQueryValues& response) const override;
  virtual bool readRecords(const QString& conditions,
                           SqlQueryValues& response) const override;
  virtual bool readLastRecord(SqlQueryValues& response) const override;
  virtual bool updateRecords(const SqlQueryValues& newValues) const override;
  virtual bool updateRecords(const QString& condition,
                             const SqlQueryValues& newValues) const override;
  virtual bool deleteRecords(const QString& condition) const override;
  virtual bool clear() const override;

  // Aggregation
  virtual bool getRecordCount(uint32_t& count) const override;

 private:
  Q_DISABLE_COPY_MOVE(PostgreSqlTable)
  void sendLog(const QString& log) const;
  void loadSettings(void);

  bool checkFieldNames(const SqlQueryValues& record) const;
  bool checkFieldNames(const QHash<QString, QString>& record) const;

 signals:
};

#endif  // SQLTABLE_H
