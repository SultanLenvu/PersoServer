#ifndef PERSOCLIENT_H
#define PERSOCLIENT_H

#include <QByteArray>
#include <QDataStream>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QSettings>
#include <QTcpSocket>
#include <QTimer>

#include "General/definitions.h"

class PersoClient : public QObject
{
  Q_OBJECT
public:
  enum ReturnStatus {
    Completed,
    FirmwareFileSavingError,
    RequestParameterError,
    ServerConnectionError,
    ServerNotResponding,
    ServerConnectionTerminated,
    ResponseParsingError,
    ResponseSyntaxError,
    AuthorizationNotExist,
    AuthorizationAccessDenied,
    AuthorizationNotActive,
    // Статусы возврата от сервера
    CommandSyntaxError,
    DatabaseError,
    TransponderNotFound,
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
  Q_ENUM(ReturnStatus)

  enum InstanceState {
    Ready,
    CreatingRequest,
    WaitingServerConnection,
    WaitingResponse,
    ProcessingResponse,
  };
  Q_ENUM(InstanceState)

private:
  bool LogEnable;
  bool ExtendedLoggingEnable;
  QHostAddress PersoServerAddress;
  uint32_t PersoServerPort;

  InstanceState CurrentState;

  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QHash<QString, ReturnStatus (PersoClient::*)(void)> ResponseHandlers;
  QHash<QString, std::shared_ptr<QVector<QString>>> ResponseTemplates;
  QJsonObject CurrentCommand;
  QJsonObject CurrentResponse;

  QHash<QString, QString>* ResponseData;
  QFile* Firmware;

  QTimer* WaitTimer;
  QEventLoop* WaitingLoop;

  QHash<QString, ReturnStatus> ServerStatusMatchTable;

public:
  explicit PersoClient(QObject* parent);
  ~PersoClient();

  ReturnStatus connectToServer(void);
  ReturnStatus disconnectFromServer(void);

  ReturnStatus requestEcho(void);
  ReturnStatus requestAuthorize(const QHash<QString, QString>* requestData);

  ReturnStatus requestTransponderRelease(const QHash<QString, QString>* requestData,
					 QFile* firmware,
					 QHash<QString, QString>* responseData);
  ReturnStatus requestTransponderReleaseConfirm(const QHash<QString, QString>* requestData);
  ReturnStatus requestTransponderRerelease(const QHash<QString, QString>* requestData,
					   QFile* firmware,
					   QHash<QString, QString>* responseData);
  ReturnStatus requestTransponderRereleaseConfirm(const QHash<QString, QString>* requestData);
  ReturnStatus requestProductionLineRollback(const QHash<QString, QString>* requestData);

  ReturnStatus requestBoxStickerPrint(const QHash<QString, QString>* requestData);
  ReturnStatus requestBoxStickerReprint();
  ReturnStatus requestPalletStickerPrint(const QHash<QString, QString>* requestData);
  ReturnStatus requestPalletStickerReprint(void);

  void applySettings(void);

private:
  Q_DISABLE_COPY_MOVE(PersoClient)
  void loadSettings(void);
  void sendLog(const QString& log);

  bool processingServerConnection(void);
  bool processingDataBlock(void);
  void createTransmittedDataBlock(void);
  ReturnStatus transmitDataBlock(void);

  void createEcho(void);
  void createAuthorization(const QHash<QString, QString>* requestData);
  void createTransponderRelease(const QHash<QString, QString>* requestData);
  void createTransponderReleaseConfirm(const QHash<QString, QString>* requestData);
  void createTransponderRerelease(const QHash<QString, QString>* requestData);
  void createTransponderRereleaseConfirm(const QHash<QString, QString>* requestData);
  void createProductionLineRollback(const QHash<QString, QString>* requestData);
  void createBoxStickerPrint(const QHash<QString, QString>* requestData);
  void createBoxStickerReprint(void);
  void createPalletStickerPrint(const QHash<QString, QString>* requestData);
  void createPalletStickerReprint(void);

  ReturnStatus processEcho(void);
  ReturnStatus processAuthorization(void);
  ReturnStatus processTransponderRelease(void);
  ReturnStatus processTransponderReleaseConfirm(void);
  ReturnStatus processTransponderRerelease(void);
  ReturnStatus processTransponderRereleaseConfirm(void);

  ReturnStatus processProductionLineRollback(void);

  ReturnStatus processPrintBoxSticker(void);
  ReturnStatus processPrintLastBoxSticker(void);
  ReturnStatus processPrintPalletSticker(void);
  ReturnStatus processPrintLastPalletSticker(void);

  void createTimers(void);
  void createSocket(void);
  void createResponseHandlers(void);
  void createResponseTemplates(void);
  void createServerStatusMatchTable(void);

private slots:
  void on_SocketConnected_slot(void);
  void on_SocketDisconnected_slot(void);

  void on_SocketReadyRead_slot(void);
  void on_SocketError_slot(QAbstractSocket::SocketError socketError);

  void on_WaitTimerTimeout_slot(void);

signals:
  void logging(const QString& log);
  void stopResponseWaiting(void);
};

#endif // PERSOCLIENT_H
