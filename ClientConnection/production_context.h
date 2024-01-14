#ifndef PRODUCTION_CONTEXT_H
#define PRODUCTION_CONTEXT_H

#include <QMutex>

#include "sql_query_values.h"

class ProductionContext {
 private:
  QMutex Mutex;

  QString Login;
  QString Password;

  SqlQueryValues ProductionLine;
  SqlQueryValues Transponder;
  SqlQueryValues Box;
  SqlQueryValues Pallet;
  SqlQueryValues Order;
  SqlQueryValues Issuer;
  SqlQueryValues MasterKeys;

 public:
  explicit ProductionContext();
  ~ProductionContext();

  const QString& login(void) const;
  void setLogin(QString& login);
  const QString& password(void) const;
  void setPassword(QString& password);
  bool isActive(void) const;
  bool isLaunched(void) const;

  SqlQueryValues& productionLine(void);
  SqlQueryValues& transponder(void);
  SqlQueryValues& box(void);
  SqlQueryValues& pallet(void);
  SqlQueryValues& order(void);
  SqlQueryValues& issuer(void);
  SqlQueryValues& masterKeys(void);

 private:
  Q_DISABLE_COPY_MOVE(ProductionContext)
};

#endif  // PRODUCTION_CONTEXT_H
