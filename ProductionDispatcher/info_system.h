#ifndef INFOSYSTEM_H
#define INFOSYSTEM_H

#include "abstract_info_system.h"

class InfoSystem : public AbstractInfoSystem {
  Q_OBJECT

 private:
  bool ContextReady;

  std::shared_ptr<SqlQueryValues> CurrentProductionLine;
  std::shared_ptr<SqlQueryValues> CurrentTransponder;
  std::shared_ptr<SqlQueryValues> CurrentBox;
  std::shared_ptr<SqlQueryValues> CurrentPallet;
  std::shared_ptr<SqlQueryValues> CurrentOrder;
  std::shared_ptr<SqlQueryValues> CurrentIssuer;
  std::shared_ptr<SqlQueryValues> CurrentMasterKeys;

 public:
  explicit InfoSystem(const QString& name,
                      const std::shared_ptr<AbstractSqlDatabase> db);
  ~InfoSystem();

  // AbstractInfoSystem interface
 public:
  virtual void setContext(const ProductionContext& context) override;

  virtual ReturnStatus generateProductionContext(
      const QString& login,
      ProductionContext& context) override;

  virtual ReturnStatus generateTransponderData(StringDictionary& data) override;
  virtual ReturnStatus generateFirmwareSeed(StringDictionary& seed) override;

  virtual ReturnStatus generateBoxData(StringDictionary& data) override;
  virtual ReturnStatus generatePalletData(StringDictionary& data) override;

  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(InfoSystem)
  void loadSettings(void);
  void sendLog(const QString& log) const;

  void initContext(ProductionContext& context);
  QString generateTransponderSerialNumber(const QString& id) const;
};

#endif  // INFOSYSTEM_H
