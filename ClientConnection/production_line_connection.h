#ifndef PersoClientConnection_H
#define PersoClientConnection_H

#include <QCoreApplication>
#include <QDataStream>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

#include "Database/database_controller.h"
#include "Database/postgres_controller.h"
#include "Management/firmware_generation_system.h"
#include "Management/transponder_release_system.h"
#include "StickerPrinter/isticker_printer.h"

class PersoClientConnection : public QObject {
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

  uint32_t Id;

  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QHash<QString, void (PersoClientConnection::*)(void)> CommandHandlers;
  QHash<QString, QSharedPointer<QVector<QString>>> CommandTemplates;
  QJsonObject CurrentCommand;
  QJsonObject CurrentResponse;

  QTimer* ExpirationTimer;
  QTimer* DataBlockWaitTimer;

  FirmwareGenerationSystem* Generator;

  QHash<TransponderReleaseSystem::ReturnStatus, ServerStatus>
      ServerStatusMatchTable;

 public:
  explicit PersoClientConnection(uint32_t id, qintptr socketDescriptor);
  ~PersoClientConnection();

  uint32_t getId(void) const;

 public slots:
  void instanceTesting(void);

 private:
  Q_DISABLE_COPY(PersoClientConnection)
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

  void authorize_signal(const QHash<QString, QString>* parameters,
                        TransponderReleaseSystem::ReturnStatus* status);
  void release_signal(const QHash<QString, QString>* parameters,
                      QHash<QString, QString>* seed,
                      QHash<QString, QString>* data,
                      TransponderReleaseSystem::ReturnStatus* status);
  void confirmRelease_signal(const QHash<QString, QString>* parameters,
                             TransponderReleaseSystem::ReturnStatus* status);
  void rerelease_signal(const QHash<QString, QString>* parameters,
                        QHash<QString, QString>* seed,
                        QHash<QString, QString>* data,
                        TransponderReleaseSystem::ReturnStatus* status);
  void confirmRerelease_signal(const QHash<QString, QString>* parameters,
                               TransponderReleaseSystem::ReturnStatus* status);
  void search_signal(const QHash<QString, QString>* parameters,
                     QHash<QString, QString>* data,
                     TransponderReleaseSystem::ReturnStatus* status);

  void productionLineRollback_signal(
      const QHash<QString, QString>* parameters,
      TransponderReleaseSystem::ReturnStatus* status);

  void getBoxData_signal(const QHash<QString, QString>* parameters,
                         QHash<QString, QString>* data,
                         TransponderReleaseSystem::ReturnStatus* status);
  void getPalletData_signal(const QHash<QString, QString>* parameters,
                            QHash<QString, QString>* data,
                            TransponderReleaseSystem::ReturnStatus* status);

  void printBoxSticker_signal(const QHash<QString, QString>* data,
                              IStickerPrinter::ReturnStatus* status);
  void printLastBoxSticker_signal(IStickerPrinter::ReturnStatus* status);
  void printPalletSticker_signal(const QHash<QString, QString>* data,
                                 IStickerPrinter::ReturnStatus* status);
  void printLastPalletSticker_signal(IStickerPrinter::ReturnStatus* status);
};

#endif  // PersoClientConnection_H
