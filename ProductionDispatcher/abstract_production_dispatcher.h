#ifndef ABSTRACTPRODUCTIONDISPATCHER_H
#define ABSTRACTPRODUCTIONDISPATCHER_H

#include <types.h>
#include <QObject>

class AbstractProductionDispatcher : public QObject {
  Q_OBJECT

 public:
  explicit AbstractProductionDispatcher(const QString& name);

 public slots:
  virtual void onInstanceThreadStarted(void) = 0;

  virtual void start(ReturnStatus& ret) = 0;
  virtual void stop(void) = 0;

  virtual void launchProductionLine(ReturnStatus& ret) = 0;
  virtual void shutdownProductionLine(ReturnStatus& ret) = 0;
  virtual void getProductinoLineData(StringDictionary& data,
                                     ReturnStatus& ret) = 0;

  virtual void requestBox(ReturnStatus& ret) = 0;
  virtual void getCurrentBoxData(StringDictionary& result,
                                 ReturnStatus& ret) = 0;
  virtual void refundBox(ReturnStatus& ret) = 0;
  virtual void completeBox(ReturnStatus& ret) = 0;

  virtual void releaseTransponder(QByteArray& firmware, ReturnStatus& ret) = 0;
  virtual void confirmTransponderRelease(const StringDictionary& param,
                                         ReturnStatus& ret) = 0;
  virtual void rereleaseTransponder(const StringDictionary& param,
                                    QByteArray& firmware,
                                    ReturnStatus& ret) = 0;
  virtual void confirmTransponderRerelease(const StringDictionary& param,
                                           ReturnStatus& ret) = 0;
  virtual void rollbackTransponder(ReturnStatus& ret) = 0;
  virtual void getCurrentTransponderData(StringDictionary& result,
                                         ReturnStatus& ret) = 0;
  virtual void getTransponderData(const StringDictionary& param,
                                  StringDictionary& result,
                                  ReturnStatus& ret) = 0;

  virtual void printBoxStickerManually(const StringDictionary& param,
                                       ReturnStatus& ret) = 0;
  virtual void printLastBoxStickerManually(ReturnStatus& ret) = 0;
  virtual void printPalletStickerManually(const StringDictionary& param,
                                          ReturnStatus& ret) = 0;
  virtual void printLastPalletStickerManually(ReturnStatus& ret) = 0;

 private:
  explicit AbstractProductionDispatcher();
  Q_DISABLE_COPY_MOVE(AbstractProductionDispatcher);

 signals:
  void logging(const QString& log);
  void errorDetected(ReturnStatus);
};

#endif  // ABSTRACTPRODUCTIONDISPATCHER_H
