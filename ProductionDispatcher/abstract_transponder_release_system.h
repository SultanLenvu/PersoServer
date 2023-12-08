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
  explicit AbstractReleaseSystem(
      const QString& name,
      std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractReleaseSystem();

  virtual ReturnStatus release(const ProductionContext& context,
                               const StringDictionary& param) = 0;
  virtual ReturnStatus confirmRelease(const ProductionContext& context,
                                      const StringDictionary& param) = 0;
  virtual ReturnStatus rerelease(const ProductionContext& context,
                                 const StringDictionary& param) = 0;
  virtual ReturnStatus confirmRerelease(const ProductionContext& context,
                                        const StringDictionary& param) = 0;
  virtual ReturnStatus rollback(const ProductionContext& context,
                                const StringDictionary& param) = 0;

 private:
  AbstractReleaseSystem();
  Q_DISABLE_COPY_MOVE(AbstractReleaseSystem)

 signals:
};

#endif  // AbstractReleaseSystem_H
