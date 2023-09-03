#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QAbstractTableModel>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVector>

#include "General/definitions.h"
#include "Management/user_settings.h"
#include "database_table_model.h"

class IDatabaseController : public QObject {
  Q_OBJECT
 public:
  enum TransactionResult { Complete, Abort };

 protected:
  bool LogOption;

 public:
  explicit IDatabaseController(QObject* parent);

  // Функционал для работы с БД
  virtual bool connect(void) = 0;
  virtual void disconnect(TransactionResult result) = 0;

  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) = 0;
  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) = 0;
  virtual void applySettings() = 0;

 protected:
  void sendLog(const QString& log) const;

 signals:
  void logging(const QString& log) const;
};

#endif  // DATABASECONTROLLER_H
