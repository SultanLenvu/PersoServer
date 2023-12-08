#ifndef AbstractLaunchSystem_H
#define AbstractLaunchSystem_H

#include <QObject>

#include <General/types.h>
#include "Database/abstract_sql_database.h"

class AbstractLaunchSystem : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<AbstractSqlDatabase> Database;

 public:
  explicit AbstractLaunchSystem(const QString& name,
                                std::shared_ptr<AbstractSqlDatabase> db);
  virtual ~AbstractLaunchSystem();

  virtual ReturnStatus init(const StringDictionary& param) const = 0;
  virtual ReturnStatus launch(const StringDictionary& param) const = 0;
  virtual ReturnStatus shutdown(const StringDictionary& param) const = 0;
  virtual bool isLaunched(const StringDictionary& param) const = 0;

 private:
  AbstractLaunchSystem();
  Q_DISABLE_COPY_MOVE(AbstractLaunchSystem)

 signals:
};

#endif  // AbstractLaunchSystem_H
