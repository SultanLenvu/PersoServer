#ifndef BOXRELEASESYSTEM_H
#define BOXRELEASESYSTEM_H

#include "abstract_box_release_system.h"

class BoxReleaseSystem : public AbstractBoxReleaseSystem
{
  Q_OBJECT

 public:
  explicit BoxReleaseSystem(const QString& name);
  ~BoxReleaseSystem();

 public:  // AbstractBoxReleaseSystem interface
  virtual ReturnStatus request(void) override;
  virtual ReturnStatus refund(void) override;
  virtual ReturnStatus complete(void) override;

 private:
  Q_DISABLE_COPY_MOVE(BoxReleaseSystem)

  ReturnStatus findBox(void);

  bool attachBox(void);
  bool detachBox(void);

  ReturnStatus startBoxAssembly(void);
  ReturnStatus startPalletAssembly(const QString& id);

  bool stopBoxAssembly(void);
  bool stopPalletAssembly(const QString& id);

  ReturnStatus completePallet(const QString& id);
  ReturnStatus completeOrder(void);

  ReturnStatus loadPalletData(const QString& id);

  bool updateProductionLine(const SqlQueryValues& newValues);
  bool updateTransponder(const SqlQueryValues& newValues);
  bool updateBox(const SqlQueryValues& newValues);
  bool updatePallet(const QString& id, const SqlQueryValues& newValues);
  bool updateOrder(const SqlQueryValues& newValues);
};

#endif // BOXRELEASESYSTEM_H
