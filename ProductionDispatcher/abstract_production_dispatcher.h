#ifndef ABSTRACTPRODUCTIONDISPATCHER_H
#define ABSTRACTPRODUCTIONDISPATCHER_H

#include <General/types.h>
#include <QObject>

class AbstractProductionDispatcher : public QObject {
  Q_OBJECT
 public:
  enum ReturnStatus {
    DatabaseQueryError,
    DatabaseTransactionError,
    DatabaseConnectionError,
    TransponderNotFound,
    FreeBoxMissed,
    TransponderNotReleasedEarlier,
    AwaitingConfirmationError,
    IdenticalUcidError,
    ProductionLineMissed,
    ProductionLineNotActive,
    CurrentOrderRunOut,
    CurrentOrderAssembled,
    ProductionLineRollbackLimitError,
    ProductionLineStopError,
    BoxStickerPrintError,
    PalletStickerPrintError,
    NextTransponderNotFound,
    StartBoxAssemblingError,
    StartPalletAssemblingError,
    Completed,
  };
  Q_ENUM(ReturnStatus)

 public:
  explicit AbstractProductionDispatcher(const QString& name);

  virtual bool checkConfiguration(void) = 0;

 public slots:
  virtual void start(ReturnStatus ret) = 0;
  virtual void stop(void) = 0;

  virtual void launchProductionLine(const StringDictionary& param,
                                    StringDictionary& context,
                                    ReturnStatus ret) = 0;
  virtual void shutdownProductionLine(const StringDictionary& param,
                                      ReturnStatus ret) = 0;
  virtual void getProductionLineContext(const StringDictionary& param,
                                        StringDictionary& result,
                                        ReturnStatus ret) = 0;
  virtual void rollbackProductionLine(const StringDictionary& param,
                                      StringDictionary& context,
                                      ReturnStatus ret) = 0;

  virtual void releaseTransponder(const StringDictionary& param,
                                  StringDictionary& seed,
                                  ReturnStatus ret) = 0;
  virtual void confirmTransponderRelease(const StringDictionary& param,
                                         StringDictionary& context,
                                         ReturnStatus ret) = 0;
  virtual void rereleaseTransponder(const StringDictionary& param,
                                    StringDictionary& seed,
                                    ReturnStatus ret) = 0;
  virtual void confirmTransponderRerelease(const StringDictionary& param,
                                           StringDictionary& context,
                                           ReturnStatus ret) = 0;

  virtual void printBoxStickerManually(const StringDictionary& param,
                                       ReturnStatus ret) = 0;
  virtual void printLastBoxStickerManually(ReturnStatus ret) = 0;
  virtual void printPalletStickerManually(const StringDictionary& param,
                                          ReturnStatus ret) = 0;
  virtual void printLastPalletStickerManually(ReturnStatus ret) = 0;

 private:
  explicit AbstractProductionDispatcher();
  Q_DISABLE_COPY_MOVE(AbstractProductionDispatcher);

 signals:
  void logging(const QString& log);
  void errorDetected(ReturnStatus);
};

#endif  // ABSTRACTPRODUCTIONDISPATCHER_H
