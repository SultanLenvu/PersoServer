#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <Database/database_controller.h>
#include <Database/postgres_controller.h>
#include <QObject>

#include "transponder_info_model.h"

class TransponderReleaseSystem : public QObject {
  Q_OBJECT

 private:
  PostgresController* Database;

 public:
  explicit TransponderReleaseSystem(QObject* parent);

  bool start(void);
  bool stop(void);
  bool release(TransponderInfoModel* seed);
  bool confirmRelease(TransponderInfoModel* seed);
  bool rerelease(TransponderInfoModel* seed);
  bool refund(TransponderInfoModel* seed);
  bool search(TransponderInfoModel* seed);

  void applySettings();

 private:
  void createDatabaseController(void);
  void loadSettings(void);

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
};

#endif // TRANSPONDERRELEASESYSTEM_H
