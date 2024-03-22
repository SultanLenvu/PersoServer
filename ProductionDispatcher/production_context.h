#ifndef PRODUCTIONDISPATCHERCONTEXT_H
#define PRODUCTIONDISPATCHERCONTEXT_H

#include <QJsonObject>

#include "abstract_context.h"
#include "sql_query_values.h"

class ProductionContext final : public AbstractContext {
 private:
  std::unordered_map<QString, std::shared_ptr<SqlQueryValues>> PalletStorage;

  std::shared_ptr<SqlQueryValues> Pallet;
  SqlQueryValues Order;
  SqlQueryValues Issuer;
  SqlQueryValues MasterKeys;

  std::unique_ptr<ProductionContext> Stash;

 public:
  ProductionContext();
  ~ProductionContext();

 public:  // AbstractContext interface
  virtual void clear(void) override;
  virtual void stash() override;
  virtual void applyStash() override;

 public:
  bool isValid(void);
  bool isOrderReady(void);

 public:
  void removePallet(const QString id);

 public:
  SqlQueryValues& pallet(const QString& id);
  SqlQueryValues& order(void);
  SqlQueryValues& issuer(void);
  SqlQueryValues& masterKeys(void);

 private:
  Q_DISABLE_COPY_MOVE(ProductionContext)
};

#endif  // PRODUCTIONDISPATCHERCONTEXT_H
