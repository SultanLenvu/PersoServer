#ifndef SQL_QUERY_VALUES_H
#define SQL_QUERY_VALUES_H

#include <QAbstractTableModel>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QSet>

#include <QSqlQuery>
#include <QVector>

#include "types.h"

class SqlQueryValues : public QAbstractTableModel {
  Q_OBJECT

 private:
  QVector<QString> Fields;
  QHash<QString, int32_t> FieldIndex;
  std::vector<SharedVector<QString>> Values;

 public:
  SqlQueryValues();
  ~SqlQueryValues();

  SqlQueryValues(const SqlQueryValues& other);
  SqlQueryValues(SqlQueryValues&& other) noexcept;
  SqlQueryValues& operator=(const SqlQueryValues& other);
  SqlQueryValues& operator=(SqlQueryValues&& other) noexcept;

 public:
  QString fieldName(uint32_t i) const;
  QString get(uint32_t record, const QString& field) const;
  QString get(uint32_t record, uint32_t field) const;
  QString get(uint32_t field) const;
  QString get(const QString& field) const;
  QString getLast(const QString& field) const;
  StringDictionary getRecord(uint32_t num) const;

  size_t recordCount(void) const;
  size_t fieldCount(void) const;
  bool isEmpty(void) const;
  void appendToInsert(QString& queryText) const;

  void extractRecords(QSqlQuery& request);
  void add(const StringDictionary& record);
  void add(const QString& field, const QString& value);
  void addField(const QString& field);
  void addField(const QString& name, const SharedVector<QString> values);
  void addRecord(const StringDictionary record);
  void clear();

  // Интерфейс модели
 public:
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
};

#endif  // SQL_QUERY_VALUES_H
