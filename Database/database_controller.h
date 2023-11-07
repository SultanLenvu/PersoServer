#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QAbstractTableModel>
#include <QHash>
#include <QHostAddress>
#include <QObject>
#include <QScopedPointer>
#include <QSettings>
#include <QStringList>
#include <QVector>
#include <QtSql>

#include "database_table_model.h"

class IDatabaseController : public QObject {
  Q_OBJECT
 protected:
  bool LogEnable;

  std::vector<QString> Tables;
  std::unordered_map<std::pair<QString, QString>, std::pair<QString, QString>>
      ForeignKeys;

  Qt::SortOrder CurrentOrder;
  uint32_t RecordsLimit;

 public:
  explicit IDatabaseController(QObject* parent);
  virtual ~IDatabaseController();

  void clearTables(void);
  void addTable(const QString& name);

  void clearForeignKeys(void);
  void addForeignKeys(const QString& table1,
                      const QString& foreignKey,
                      const QString& table2,
                      const QString& primaryKey);

  Qt::SortOrder getCurrentOrder() const;
  void setCurrentOrder(Qt::SortOrder newCurrentOrder);

  uint32_t getRecordsLimit() const;
  void setRecordsLimit(uint32_t newRecordsLimit);

  virtual bool connect(void) = 0;
  virtual void disconnect(void) = 0;
  virtual bool isConnected(void) = 0;

  virtual bool openTransaction(void) const = 0;
  virtual bool closeTransaction(void) const = 0;
  virtual bool abortTransaction(void) const = 0;

  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) const = 0;
  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) const = 0;
  virtual bool clearTable(const QString& tableName) const = 0;

  virtual bool addRecord(const QString& tableName,
                         QHash<QString, QString>& record) const = 0;

  virtual bool getRecordById(const QString& tableName,
                             QHash<QString, QString>& record) const = 0;
  virtual bool getRecordByPart(const QString& tableName,
                               QHash<QString, QString>& record,
                               bool order = true) const = 0;
  virtual bool getLastRecord(const QString& tableName,
                             QHash<QString, QString>& record) const = 0;
  virtual bool getMergedRecordById(const QStringList& tables,
                                   const QStringList& foreignKeys,
                                   QHash<QString, QString>& record) const = 0;
  virtual bool getMergedRecordByPart(const QStringList& tables,
                                     const QStringList& foreignKeys,
                                     QHash<QString, QString>& record) const = 0;
  //  virtual bool getAllMergedRecords(
  //      const QStringList& tableNames,
  //      const QStringList& foreignKeys,
  //      const QHash<QString, QString>& searchValue,
  //      QVector<QScopedPointer<QHash<QString, QString>>>& records) const = 0;

  virtual bool updateRecordById(const QString& tableName,
                                QHash<QString, QString>& record) const = 0;
  virtual bool removeRecordById(const QString& tableName,
                                const uint32_t id) const = 0;
  virtual bool removeLastRecordById(const QString& tableName) const = 0;
  virtual bool removeLastRecordByCondition(
      const QString& tableName,
      QHash<QString, QString>& condition) const = 0;

  virtual void applySettings() = 0;

 protected:
  void sendLog(const QString& log) const;

 private:
  Q_DISABLE_COPY_MOVE(IDatabaseController)

 signals:
  void logging(const QString& log);
  void disconnected(void);
};

#endif  // DATABASECONTROLLER_H
