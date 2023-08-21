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

class DatabaseControllerInterface : public QObject {
  Q_OBJECT

 public:
  explicit DatabaseControllerInterface(QObject* parent);

  // Функционал для работы с БД
  virtual void connect(void) = 0;
  virtual void disconnect(void) = 0;

  virtual void getObuByPAN(const QString& pan, DatabaseBuffer* buffer) = 0;
  virtual void getObuBySerialNumber(const uint32_t serial,
                                    DatabaseBuffer* buffer) = 0;
  virtual void getObuByUCID(const QString& ucid, DatabaseBuffer* buffer) = 0;

  virtual void getObuListByContextMark(const QString& cm,
                                       DatabaseBuffer* buffer) = 0;
  virtual void getObuListBySerialNumber(const uint32_t serialBegin,
                                        const uint32_t serialEnd,
                                        DatabaseBuffer* buffer) = 0;
  virtual void getObuListByPAN(const uint32_t panBegin,
                               const uint32_t panEnd,
                               DatabaseBuffer* buffer) = 0;

  virtual void getTable(const QString& tableName,
                        uint32_t rowCount,
                        DatabaseBuffer* buffer) = 0;

  virtual void execCustomRequest(const QString& req,
                                 DatabaseBuffer* buffer) = 0;

  virtual void applySettings(QSettings* settings) = 0;

 signals:
  void logging(const QString& log);
};

#endif  // DATABASECONTROLLER_H
