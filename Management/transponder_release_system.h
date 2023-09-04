#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <Database/database_controller.h>
#include <QObject>

#include "transponder_info_model.h"

class TransponderReleaseSystem : public QObject {
  Q_OBJECT

 private:
  IDatabaseController* Database;

 public:
  explicit TransponderReleaseSystem(QObject* parent,
                                    IDatabaseController* database);

  bool getSeed(const TransponderInfoModel& seed);
  bool confirmRelease(const TransponderInfoModel& seed);
  bool refund(const TransponderInfoModel& seed);

  void applySettings();

 private:
  void loadSettings(void);

 private slots:
  void proxyLogging(const QString& log) const;

 signals:
  void logging(const QString& log) const;
};

#endif // TRANSPONDERRELEASESYSTEM_H
