#ifndef PRODUCTION_LINE_CONTEXT_H
#define PRODUCTION_LINE_CONTEXT_H

#include <QJsonObject>

#include "abstract_context.h"
#include "sql_query_values.h"

class ProductionLineContext final : AbstractContext {
 private:
  QString Login;
  QString Password;

  SqlQueryValues ProductionLine;
  SqlQueryValues Transponder;
  SqlQueryValues Box;

  std::unique_ptr<ProductionLineContext> Stash;

 public:
  ProductionLineContext();
  ~ProductionLineContext();

 public:  // AbstractContext interface
  virtual void clear(void) override;
  virtual void stash(void) override;
  virtual void applyStash(void) override;

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

 private:
  Q_DISABLE_COPY_MOVE(ProductionLineContext)

  QString generateTransponderSerialNumber(const QString& id) const;
};

#endif  // PRODUCTION_LINE_CONTEXT_H
