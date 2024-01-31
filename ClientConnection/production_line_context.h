#ifndef PRODUCTION_LINE_CONTEXT_H
#define PRODUCTION_LINE_CONTEXT_H

#include <QJsonObject>

#include "sql_query_values.h"

class ProductionLineContext final {
 private:
  QString Login;
  QString Password;

  SqlQueryValues ProductionLine;
  SqlQueryValues Transponder;
  SqlQueryValues Box;
  SqlQueryValues Pallet;
  SqlQueryValues Order;
  SqlQueryValues Issuer;
  SqlQueryValues MasterKeys;

  std::unique_ptr<ProductionLineContext> Stash;

 public:
  ProductionLineContext();
  ~ProductionLineContext();

 public:
  const QString& login(void) const;
  void setLogin(const QString& login);

  const QString& password(void) const;
  void setPassword(const QString& password);

 public:
  bool isActive(void) const;
  bool isLaunched(void) const;
  bool isAuthorized(void) const;
  bool isInProcess(void) const;

 public:
  SqlQueryValues& productionLine(void);
  SqlQueryValues& transponder(void);
  SqlQueryValues& box(void);
  SqlQueryValues& pallet(void);
  SqlQueryValues& order(void);
  SqlQueryValues& issuer(void);
  SqlQueryValues& masterKeys(void);

 public:
  void stash(void);
  void applyStash(void);

 public:
  void generateFirmwareSeed(StringDictionary seed) const;

  void addTransponderDataToJson(QJsonObject& json) const;
  void addBoxDataToJson(QJsonObject& json) const;

 private:
  //  Q_DISABLE_COPY_MOVE(ProductionLineContext)

  QString generateTransponderSerialNumber(const QString& id) const;
};

#endif  // PRODUCTION_LINE_CONTEXT_H
