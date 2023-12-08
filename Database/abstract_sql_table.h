#ifndef ABSTRACTSQLTABLE_H
#define ABSTRACTSQLTABLE_H

#include <QObject>

#include "sql_query_values.h"

class AbstractSqlTable : public QObject {
  Q_OBJECT

 public:
  explicit AbstractSqlTable(const QString& name);
  virtual ~AbstractSqlTable();

  virtual bool init() = 0;
  virtual void applySettings() = 0;

  // Create
  virtual bool createRecords(const SqlQueryValues& records) const = 0;

  // Read
  virtual bool readRecords(SqlQueryValues& records) const = 0;
  virtual bool readRecords(const QString& conditions,
                           SqlQueryValues& records) const = 0;
  virtual bool readLastRecord(SqlQueryValues& record) const = 0;

  // Update
  virtual bool updateRecords(const SqlQueryValues& newValues) const = 0;
  virtual bool updateRecords(const QString& condition,
                             const SqlQueryValues& newValues) const = 0;

  // Delete
  virtual bool deleteRecords(const QString& condition) const = 0;
  virtual bool clear(void) const = 0;

  // Aggregation
  virtual bool getRecordCount(uint32_t& count) const = 0;

 private:
  AbstractSqlTable();
  Q_DISABLE_COPY_MOVE(AbstractSqlTable)

 signals:
};

#endif  // ABSTRACTSQLTABLE_H
