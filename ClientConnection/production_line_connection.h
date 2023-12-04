#ifndef PRODUCTION_LINE_CONNECTION_H
#define PRODUCTION_LINE_CONNECTION_H

#include <QCoreApplication>
#include <QDataStream>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

#include "ClientConnection/abstract_client_connection.h"
#include "ClientConnection/abstract_firmware_generation_system.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"

class ProductionLineConnection : public AbstractClientConnection {
  Q_OBJECT

 private:
  enum ServerStatus {
    NoError = 0,
    CommandSyntaxError,
    DatabaseError,
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
    BoxStickerPrintError,
    PalletStickerPrintError,
    NextTransponderNotFound,
    StartBoxAssemblingError,
    StartPalletAssemblingError,
  };
  //  Q_ENUM(ServerStatus);

 private:
  bool LogEnable;
  bool ExtendedLogEnable;
  int32_t MaximumConnectionTime;

  size_t Id;

  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QHash<QString, void (ProductionLineConnection::*)(void)> CommandHandlers;
  QHash<QString, QSharedPointer<QVector<QString>>> CommandTemplates;
  QJsonObject CurrentCommand;
  QJsonObject CurrentResponse;

  std::unique_ptr<QTimer> ExpirationTimer;
  std::unique_ptr<QTimer> DataBlockWaitTimer;

  std::unique_ptr<AbstractFirmwareGenerationSystem> Generator;

  QHash<AbstractProductionDispatcher::ReturnStatus, ServerStatus>
      ServerStatusMatchTable;

 public:
  explicit ProductionLineConnection(const QString& name,
                                    uint32_t id,
                                    qintptr socketDescriptor);
  ~ProductionLineConnection();

  virtual size_t getId(void) const override;

 private:
  ProductionLineConnection();
  Q_DISABLE_COPY(ProductionLineConnection)
  void loadSettings(void);
  void sendLog(const QString& log);

  // Блоки данных
  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  void processReceivedDataBlock(void);

  // Обработчики команд
  void processSyntaxError(void);
  void processEcho(void);
  void processAuthorization(void);

  void processTransponderRelease(void);
  void processTransponderReleaseConfirm(void);
  void processTransponderRerelease(void);
  void processTransponderRereleaseConfirm(void);

  void processProductionLineRollback(void);

  void processPrintBoxSticker(void);
  void processPrintLastBoxSticker(void);
  void processPrintPalletSticker(void);
  void processPrintLastPalletSticker(void);

  // Фабричные методы
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createGenerator(void);
  void createCommandHandlers(void);
  void createCommandTemplates(void);
  void createServerStatusMatchTable(void);

 private slots:
  void socketReadyRead_slot(void);
  void socketDisconnected_slot(void);
  void socketError_slot(QAbstractSocket::SocketError socketError);

  void expirationTimerTimeout_slot(void);
  void dataBlockWaitTimerTimeout_slot(void);

 signals:
  void logging(const QString& log);
  void disconnected(void);

  void authorize_signal(const StringDictionary& parameters,
                        AbstractProductionDispatcher::ReturnStatus* status);
  void release_signal(const StringDictionary& parameters,
                      StringDictionary& seed,
                      StringDictionary& data,
                      AbstractProductionDispatcher::ReturnStatus* status);
  void confirmRelease_signal(
      const StringDictionary& parameters,
      AbstractProductionDispatcher::ReturnStatus* status);
  void rerelease_signal(const StringDictionary& parameters,
                        StringDictionary& seed,
                        StringDictionary& data,
                        AbstractProductionDispatcher::ReturnStatus* status);
  void confirmRerelease_signal(
      const StringDictionary& parameters,
      AbstractProductionDispatcher::ReturnStatus* status);
  void search_signal(const StringDictionary& parameters,
                     StringDictionary& data,
                     AbstractProductionDispatcher::ReturnStatus* status);

  void productionLineRollback_signal(
      const StringDictionary& parameters,
      AbstractProductionDispatcher::ReturnStatus* status);

  void getBoxData_signal(const StringDictionary& parameters,
                         StringDictionary& data,
                         AbstractProductionDispatcher::ReturnStatus* status);
  void getPalletData_signal(const StringDictionary& parameters,
                            StringDictionary& data,
                            AbstractProductionDispatcher::ReturnStatus* status);

  void printBoxSticker_signal(
      const StringDictionary& data,
      AbstractProductionDispatcher::ReturnStatus* status);
  void printLastBoxSticker_signal(
      AbstractProductionDispatcher::ReturnStatus* status);
  void printPalletSticker_signal(
      const StringDictionary& data,
      AbstractProductionDispatcher::ReturnStatus* status);
  void printLastPalletSticker_signal(
      AbstractProductionDispatcher::ReturnStatus* status);
};

#endif  // PRODUCTION_LINE_CONNECTION_H
