#ifndef PERSOCLIENTCONNECTION_H
#define PERSOCLIENTCONNECTION_H

#include <QApplication>
#include <QDataStream>
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

class PersoClientConnection : public QObject {
  Q_OBJECT

 private:
  int32_t MaximumConnectionTime;
  bool ExtendedLoggingEnable;

  uint32_t Id;

  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QMap<QString, QJsonObject> CommandTemplates;
  QJsonObject CurrentCommand;
  QJsonObject CurrentResponse;

  QTimer* ExpirationTimer;
  QTimer* DataBlockWaitTimer;

  QTimer* ReleaserWaitTimer;
  QEventLoop* ReleaserWaiting;

  FirmwareGenerationSystem* Generator;

 public:
  explicit PersoClientConnection(uint32_t id, qintptr socketDescriptor);
  ~PersoClientConnection();

  uint32_t getId(void);

 public slots:
  void applySettings(void);
  void instanceTesting(void);
  void releaserFinished(void);

 private:
  void loadSettings(void);
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createReleaserWaitTimer(void);
  void createGenerator(void);
  void createCommandTemplates(void);

  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  void processReceivedDataBlock(void);

  void processEcho(QJsonObject* commandJson);
  void processAuthorization(QJsonObject* commandJson);
  void processTransponderRelease(QJsonObject* commandJson);
  void processTransponderReleaseConfirm(QJsonObject* commandJson);
  void processTransponderRerelease(QJsonObject* commandJson);
  void processTransponderRereleaseConfirm(QJsonObject* commandJson);

 private slots:
  void proxyLogging(const QString& log);
  void on_SocketReadyRead_slot(void);
  void on_SocketDisconnected_slot(void);
  void on_SocketError_slot(QAbstractSocket::SocketError socketError);

  void on_ExpirationTimerTimeout_slot(void);
  void on_DataBlockWaitTimerTimeout_slot(void);
  void on_ReleaserWaitTimerTimeout_slot(void);

 signals:
  void disconnected(void);
  void logging(const QString& log);

  void releaserAuthorize_signal(const QMap<QString, QString>* parameters,
                                TransponderReleaseSystem::ReturnStatus* status);
  void releaseRelease_signal(const QMap<QString, QString>* parameters,
                             QMap<QString, QString>* attributes,
                             QMap<QString, QString>* masterKeys,
                             TransponderReleaseSystem::ReturnStatus* status);
  void releaserConfirmRelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* transponderInfo,
      TransponderReleaseSystem::ReturnStatus* status);
  void releaserRerelease_signal(const QMap<QString, QString>* parameters,
                                QMap<QString, QString>* attributes,
                                QMap<QString, QString>* masterKeys,
                                TransponderReleaseSystem::ReturnStatus* status);
  void releaserConfirmRerelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* transponderInfo,
      TransponderReleaseSystem::ReturnStatus* status);
  void releaserSearch_signal(const QMap<QString, QString>* parameters,
                             QMap<QString, QString>* attributes,
                             QMap<QString, QString>* masterKeys,
                             TransponderReleaseSystem::ReturnStatus* status);
};

#endif  // PERSOCLIENTCONNECTION_H
