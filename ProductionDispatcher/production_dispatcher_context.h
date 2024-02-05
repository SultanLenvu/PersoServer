#ifndef PRODUCTIONDISPATCHERCONTEXT_H
#define PRODUCTIONDISPATCHERCONTEXT_H

#include <QJsonObject>

#include "sql_query_values.h"

class ProductionDispatcherContext final {
 private:
  std::unordered_map<QString, SqlQueryValues> Pallets;
  SqlQueryValues Order;
  SqlQueryValues Issuer;
  SqlQueryValues MasterKeys;

  std::unique_ptr<ProductionDispatcherContext> Stash;

 public:
  ProductionDispatcherContext();
  ~ProductionDispatcherContext();

  void addPallet(const SqlQueryValues& pallet);
  void removePallet(const QString id);

  SqlQueryValues& pallet(const QString& id);
  SqlQueryValues& order(void);
  SqlQueryValues& issuer(void);
  SqlQueryValues& masterKeys(void);

 private:
  Q_DISABLE_COPY_MOVE(ProductionDispatcherContext)
};

#endif  // PRODUCTIONDISPATCHERCONTEXT_H
