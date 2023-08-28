#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QAbstractTableModel>
#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVector>

#include "General/definitions.h"
#include "Management/user_settings.h"
#include "database_buffer.h"

class IDatabaseController : public QObject {
  Q_OBJECT

 public:
  explicit IDatabaseController(QObject* parent);

  // Функционал для работы с БД
  virtual bool connect(void) = 0;
  virtual void disconnect(void) = 0;
  virtual bool isConnected(void) = 0;

  virtual void getObuByPAN(const QString& pan, DatabaseTableModel* buffer) = 0;
  virtual void getObuBySerialNumber(const uint32_t serial,
                                    DatabaseTableModel* buffer) = 0;
  virtual void getObuByUCID(const QString& ucid,
                            DatabaseTableModel* buffer) = 0;

  virtual void getObuListByContextMark(const QString& cm,
                                       DatabaseTableModel* buffer) = 0;
  virtual void getObuListBySerialNumber(const uint32_t serialBegin,
                                        const uint32_t serialEnd,
                                        DatabaseTableModel* buffer) = 0;
  virtual void getObuListByPAN(const uint32_t panBegin,
                               const uint32_t panEnd,
                               DatabaseTableModel* buffer) = 0;

  virtual bool getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseTableModel* buffer) = 0;

  virtual bool execCustomRequest(const QString& req,
                                 DatabaseTableModel* buffer) = 0;

  virtual void applySettings() = 0;

 signals:
  void logging(const QString& log) const;
};

#endif  // DATABASECONTROLLER_H
