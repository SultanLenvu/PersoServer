#ifndef ABSTRACTBOXRELEASESYSTEM_H
#define ABSTRACTBOXRELEASESYSTEM_H

#include <QObject>

#include "abstract_sql_database.h"
#include "production_line_context.h"
#include "types.h"

class AbstractBoxReleaseSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionLineContext> Context;
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractBoxReleaseSystem(const QString& name,
                                    std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractBoxReleaseSystem();

  virtual void setContext(std::shared_ptr<ProductionLineContext> context) = 0;

  virtual ReturnStatus request(void) = 0;
  virtual ReturnStatus refund(void) = 0;
  virtual ReturnStatus complete(void) = 0;
  virtual void clearContext(void) = 0;

 private:
  AbstractBoxReleaseSystem();
  Q_DISABLE_COPY_MOVE(AbstractBoxReleaseSystem)

 signals:
  void logging(const QString& log);
};

#endif // ABSTRACTBOXRELEASESYSTEM_H
