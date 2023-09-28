#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QAbstractTableModel>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVector>

#include "General/definitions.h"
#include "database_table_model.h"

class IDatabaseController : public QObject {
  Q_OBJECT

 protected:
  bool LogOption;

 public:
  explicit IDatabaseController(QObject* parent);

  virtual bool connect(void) = 0;
  virtual void disconnect(void) = 0;

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
                         QMap<QString, QString>& record) const = 0;

  virtual bool getRecordById(const QString& tableName,
                             QMap<QString, QString>& record) const = 0;
  virtual bool getRecordByPart(const QString& tableName,
                               QMap<QString, QString>& record) const = 0;
  virtual bool getLastRecord(const QString& tableName,
                             QMap<QString, QString>& record) const = 0;
  virtual bool getMergedRecordById(const QStringList& tables,
                                   const QStringList& foreignKeys,
                                   QMap<QString, QString>& record) const = 0;
  virtual bool getMergedRecordByPart(const QStringList& tables,
                                     const QStringList& foreignKeys,
                                     QMap<QString, QString>& record) const = 0;

  virtual bool updateRecordById(const QString& tableName,
                                QMap<QString, QString>& record) const = 0;
  virtual bool removeRecordById(const QString& tableName,
                                const uint32_t id) const = 0;
  virtual bool removeLastRecordById(const QString& tableName) const = 0;
  virtual bool removeLastRecordByCondition(
      const QString& tableName,
      QMap<QString, QString>& condition) const = 0;

  virtual void applySettings() = 0;

 protected:
  void sendLog(const QString& log) const;

 signals:
  void logging(const QString& log) const;
};

#endif  // DATABASECONTROLLER_H
