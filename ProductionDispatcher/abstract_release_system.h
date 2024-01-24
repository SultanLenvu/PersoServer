#ifndef AbstractReleaseSystem_H
#define AbstractReleaseSystem_H

#include <QObject>

#include "abstract_sql_database.h"
#include "production_line_context.h"
#include "types.h"

class AbstractReleaseSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionLineContext> Context;
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractReleaseSystem(const QString& name,
                                 std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractReleaseSystem();

  virtual void setContext(std::shared_ptr<ProductionLineContext> context) = 0;

  virtual ReturnStatus release(void) = 0;
  virtual ReturnStatus confirmRelease(const QString& ucid) = 0;
  virtual ReturnStatus rerelease(const QString& key, const QString& value) = 0;
  virtual ReturnStatus confirmRerelease(const QString& key,
                                        const QString& value,
                                        const QString& ucid) = 0;
  virtual ReturnStatus rollback(void) = 0;

 private:
  AbstractReleaseSystem();
  Q_DISABLE_COPY_MOVE(AbstractReleaseSystem)

 signals:
  void logging(const QString& log);
};

#endif  // AbstractReleaseSystem_H
