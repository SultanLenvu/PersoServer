#ifndef INFOSYSTEM_H
#define INFOSYSTEM_H

#include "abstract_info_system.h"

class InfoSystem : public AbstractInfoSystem {
  Q_OBJECT

 private:
  std::shared_ptr<SqlQueryValues> CurrentProductionLine;
  std::shared_ptr<SqlQueryValues> CurrentTransponder;
  std::shared_ptr<SqlQueryValues> CurrentBox;
  std::shared_ptr<SqlQueryValues> CurrentPallet;
  std::shared_ptr<SqlQueryValues> CurrentOrder;
  std::shared_ptr<SqlQueryValues> CurrentIssuer;
  std::shared_ptr<SqlQueryValues> CurrentMasterKeys;

  ProductionLineContext StashedContext;

 public:
  explicit InfoSystem(const QString& name,
                      const std::shared_ptr<AbstractSqlDatabase> db);
  ~InfoSystem();

  // AbstractInfoSystem interface
 public:
  virtual void setContext(const ProductionLineContext& context) override;

  virtual ReturnStatus loadProductionLineContext(
      const QString& login,
      ProductionLineContext& context) override;

  virtual QString getTransponderBoxId(const QString& key,
                                      const QString& value) override;
  virtual QString getTransponderPalletId(const QString& key,
                                         const QString& value) override;

  virtual ReturnStatus generateTransponderData(StringDictionary& data) override;
  virtual ReturnStatus generateTransponderData(const QString& key,
                                               const QString& value,
                                               StringDictionary& data) override;
  virtual ReturnStatus generateFirmwareSeed(StringDictionary& seed) override;
  virtual ReturnStatus generateFirmwareSeed(const QString& key,
                                            const QString& value,
                                            StringDictionary& seed) override;

  virtual ReturnStatus generateBoxData(StringDictionary& data) override;
  virtual ReturnStatus generateBoxData(const QString& id,
                                       StringDictionary& data) override;
  virtual ReturnStatus generatePalletData(StringDictionary& data) override;
  virtual ReturnStatus generatePalletData(const QString& id,
                                          StringDictionary& data) override;

  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(InfoSystem)
  void loadSettings(void);
  void sendLog(const QString& log) const;

  void stashCurrentContext(void);

  ReturnStatus loadTransponderContext(const QString& key, const QString& value);
  ReturnStatus loadBoxContext(const QString& id);
  ReturnStatus loadPalletContext(const QString& id);

  void initContext(ProductionLineContext& context);
  QString generateTransponderSerialNumber(const QString& id) const;
};

#endif  // INFOSYSTEM_H
