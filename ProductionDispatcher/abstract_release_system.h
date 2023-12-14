#ifndef AbstractReleaseSystem_H
#define AbstractReleaseSystem_H

#include <QObject>

#include "Database/abstract_sql_database.h"
#include "General/types.h"

class AbstractReleaseSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractReleaseSystem(const QString& name,
                                 std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractReleaseSystem();

  virtual void setContext(const ProductionLineContext& context) = 0;
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
  void boxAssemblyCompleted(const std::shared_ptr<QString> id);
  void palletAssemblyCompleted(const std::shared_ptr<QString> id);
  void orderAssemblyCompleted(const std::shared_ptr<QString> id);
};

#endif  // AbstractReleaseSystem_H
