#ifndef ABSTRACTTRANSPONDERRELEASESYSTEM_H
#define ABSTRACTTRANSPONDERRELEASESYSTEM_H

#include <QObject>

#include "abstract_sql_database.h"
#include "production_line_context.h"
#include "types.h"

class AbstractTransponderReleaseSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionLineContext> Context;
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractTransponderReleaseSystem(
      const QString& name,
      std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractTransponderReleaseSystem();

  virtual void setContext(std::shared_ptr<ProductionLineContext> context) = 0;

  virtual ReturnStatus findLastReleased(void) = 0;
  virtual ReturnStatus findNext(void) = 0;
  virtual ReturnStatus release(void) = 0;
  virtual ReturnStatus confirmRelease(const QString& ucid) = 0;
  virtual ReturnStatus rerelease(const QString& key, const QString& value) = 0;
  virtual ReturnStatus confirmRerelease(const QString& key,
                                        const QString& value,
                                        const QString& ucid) = 0;
  virtual ReturnStatus rollback(void) = 0;

 private:
  AbstractTransponderReleaseSystem();
  Q_DISABLE_COPY_MOVE(AbstractTransponderReleaseSystem)

 signals:
  void logging(const QString& log);
};

#endif  // ABSTRACTTRANSPONDERRELEASESYSTEM_H
