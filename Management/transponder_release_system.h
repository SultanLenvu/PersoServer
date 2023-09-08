#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <QDateTime>
#include <QMutex>
#include <QObject>

#include <Database/database_controller.h>
#include <Database/postgres_controller.h>

class TransponderReleaseSystem : public QObject {
  Q_OBJECT

 private:
  bool OrderAssembled;
  bool OrderTranspondersOut;
  PostgresController* Database;

  QMutex Mutex;

 public:
  explicit TransponderReleaseSystem(QObject* parent);

  bool start(void);
  bool stop(void);
  void beginAssemblingNewOrder(const QString& id);
  void beginAssemblingNextOrder(void);
  void applySettings();

 public slots:
  void release(const QMap<QString, QString>* searchData,
               QMap<QString, QString>* resultData,
               bool& ok);
  void confirm(const QMap<QString, QString>* searchData, bool& ok);
  void rerelease(const QMap<QString, QString>* searchData,
                 QMap<QString, QString>* resultData,
                 bool& ok);
  void refund(const QMap<QString, QString>* searchData, bool& ok);
  void search(const QMap<QString, QString>* searchData,
              QMap<QString, QString>* resultData,
              bool& ok);

 private:
  void createDatabaseController(void);
  void loadSettings(void);

 private:
  bool getTranponderData(const QString& key,
                         const QString& value,
                         QMap<QString, QString>* resultData);

  bool confirmTransponder(const QString& transponderId) const;
  bool confirmBox(const QString& boxId) const;
  bool confirmPallet(const QString& palletId) const;
  bool confirmOrder(const QString& orderId) const;

  bool searchNextTransponderForAssembling(
      QMap<QString, QString>* productionLineRecord) const;

  bool refundTransponder(const QString& id) const;
  bool refundBox(const QString& id) const;
  bool refundPallet(const QString& id) const;
  bool refundOrder(const QString& id) const;

 private slots:
  void proxyLogging(const QString& log) const;
  void on_OrderAssemblingCompleted_slot(
      const QMap<QString, QString>* orderData);
  void on_OrderTranspondersOut_slot(void);

 signals:
  void logging(const QString& log) const;

  void transponderAssemblingCompleted(
      const QMap<QString, QString>* boxData) const;
  void boxAssemblingCompleted(const QMap<QString, QString>* boxData) const;
  void palletAssemblingCompleted(
      const QMap<QString, QString>* palletData) const;
  void orderAssemblingCompleted(const QMap<QString, QString>* orderData) const;
  void orderTranspondersOut(void) const;
};

#endif // TRANSPONDERRELEASESYSTEM_H
