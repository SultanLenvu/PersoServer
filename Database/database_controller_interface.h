#ifndef DATABASECONTROLLER_H
#define DATABASECONTROLLER_H

#include <QAbstractTableModel>
#include <QObject>
#include <QStringList>
#include <QVector>

#include "General/definitions.h"
#include "Management/user_settings.h"

class DatabaseControllerInterface : public QObject {
  Q_OBJECT

 public:
  explicit DatabaseControllerInterface(QObject* parent);

  // Функционал для работы с БД
  virtual void connect(void) = 0;
  virtual void disconnect(void) = 0;

  virtual void getObuByPAN(const QString& pan, QVector<QString>* result) = 0;
  virtual void getObuBySerialNumber(const uint32_t serial,
                                    QVector<QString>* result) = 0;
  virtual void getObuByUCID(const QString& ucid, QVector<QString>* result) = 0;

  virtual void getObuListByContextMark(const QString& cm,
                                       QVector<QVector<QString>>* result) = 0;
  virtual void getObuListBySerialNumber(const uint32_t serialBegin,
                                        const uint32_t serialEnd,
                                        QVector<QVector<QString>>* result) = 0;
  virtual void getObuListByPAN(const uint32_t panBegin,
                               const uint32_t panEnd,
                               QVector<QVector<QString>>* result) = 0;

  virtual void execCustomRequest(const QString& req,
                                 QVector<QVector<QString>>* result) = 0;

  virtual void applySettings(UserSettings* settings) = 0;

 signals:
  void logging(const QString& log);
};

#endif  // DATABASECONTROLLER_H
