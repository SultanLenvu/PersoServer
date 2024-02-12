#ifndef PRODUCTIONLINECONTEXTOWNER_H
#define PRODUCTIONLINECONTEXTOWNER_H

#include <QObject>

#include "production_line_context.h"

class ProductionLineContextOwner : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionLineContext> Context;

 public:
  explicit ProductionLineContextOwner(const QString& name);
  virtual ~ProductionLineContextOwner();

  std::shared_ptr<ProductionLineContext> context(void) const;
  void setContext(std::shared_ptr<ProductionLineContext> context);

 private:
  ProductionLineContextOwner();
  Q_DISABLE_COPY_MOVE(ProductionLineContextOwner)
 signals:
};

#endif  // PRODUCTIONLINECONTEXTOWNER_H
