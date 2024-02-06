#ifndef INFOSYSTEM_H
#define INFOSYSTEM_H

#include "abstract_info_system.h"

class InfoSystem : public AbstractInfoSystem {
  Q_OBJECT

 private:
  std::shared_ptr<ProductionContext> SavedMainContext;
  std::shared_ptr<ProductionLineContext> SavedSubContext;

 public:
  explicit InfoSystem(const QString& name);
  ~InfoSystem();

 public:  // AbstractInfoSystem interface
  virtual ReturnStatus updateMainContext(void) override;

  virtual ReturnStatus generateProductionLineData(
      StringDictionary& data) override;

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

  virtual QString getTransponderBoxId(const QString& key,
                                      const QString& value) override;
  virtual QString getTransponderPalletId(const QString& key,
                                         const QString& value) override;

 private:
  Q_DISABLE_COPY_MOVE(InfoSystem)

  void saveContexts(void);
  void restoreSavedContexts(void);

  ReturnStatus generatePalletDataSubprocess(const QString& id,
                                            StringDictionary& data);

  ReturnStatus loadTransponderContext(const QString& key, const QString& value);
  ReturnStatus loadBoxContext(const QString& id);
  ReturnStatus loadPalletContext(const QString& id);

  QString generateTransponderSerialNumber(const QString& id) const;
};

#endif  // INFOSYSTEM_H
