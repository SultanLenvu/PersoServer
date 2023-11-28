#ifndef SQL_QUERY_VALUES_H
#define SQL_QUERY_VALUES_H

#include <QAbstractTableModel>
#include <QHash>
#include <QMutex>
#include <QObject>
#include <QSet>

#include <QSqlQuery>
#include <QVector>

class SqlQueryValues : public QAbstractTableModel {
  Q_OBJECT

 private:
  template <typename T>
  using SharedVector = std::shared_ptr<QVector<T>>;

 private:
  QVector<QString> Fields;
  QHash<QString, int32_t> FieldIndex;
  QVector<SharedVector<QString>> Values;

  QMutex Mutex;

 public:
  explicit SqlQueryValues(QObject* parent = nullptr);
  ~SqlQueryValues();

  QString fieldName(uint32_t i) const;
  QString get(uint32_t record, const QString& field) const;
  QString get(uint32_t record, uint32_t field) const;
  QString get(uint32_t field) const;
  QString get(const QString& field) const;
  QString getLast(const QString& field) const;
  int32_t recordCount(void) const;
  int32_t fieldCount(void) const;
  bool isEmpty(void) const;
  void appendToInsert(QString& queryText) const;

  void extractRecords(QSqlQuery& request);
  void add(const QHash<QString, QString>& record);
  void add(const QString& name, const std::shared_ptr<QVector<QString>>& values);
  void add(const QString& field, const QString& value);
  void addField(const QString& field);
  void clear();

  // Интерфейс модели
 public:
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  Q_DISABLE_COPY_MOVE(SqlQueryValues)
 signals:
};

#endif  // SQL_QUERY_VALUES_H
