#ifndef PRODUCTIONCONTEXTOWNER_H
#define PRODUCTIONCONTEXTOWNER_H

#include <QObject>

#include "production_line_context.h"

class ProductionContextOwner : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionLineContext> Context;

 public:
  explicit ProductionContextOwner(const QString& name);
  virtual ~ProductionContextOwner();

  std::shared_ptr<ProductionLineContext> context(void) const;
  void setContext(std::shared_ptr<ProductionLineContext> context);

 private:
  ProductionContextOwner();
  Q_DISABLE_COPY_MOVE(ProductionContextOwner)
 signals:
};

#endif  // PRODUCTIONCONTEXTOWNER_H
