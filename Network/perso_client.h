#ifndef PersoClient_H
#define PersoClient_H

#include <QCoreApplication>
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

class PersoClient : public QObject {
  Q_OBJECT

 private:
  bool LogEnable;
  bool ExtendedLogEnable;
  int32_t MaximumConnectionTime;

  uint32_t Id;

  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QMap<QString, QSharedPointer<QVector<QString>>> CommandTemplates;
  QJsonObject CurrentCommand;
  QJsonObject CurrentResponse;

  QTimer* ExpirationTimer;
  QTimer* DataBlockWaitTimer;

  QTimer* ReleaserWaitTimer;
  QEventLoop* ReleaserWaiting;

  FirmwareGenerationSystem* Generator;

 public:
  explicit PersoClient(uint32_t id, qintptr socketDescriptor);
  ~PersoClient();

  uint32_t getId(void) const;

 public slots:
  void instanceTesting(void);
  void releaserFinished(void);

 private:
  Q_DISABLE_COPY(PersoClient);
  void loadSettings(void);
  void sendLog(const QString& log) const;

  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  void processReceivedDataBlock(void);

  void processEcho(QJsonObject* commandJson);
  void processAuthorization(QJsonObject* commandJson);
  void processTransponderRelease(QJsonObject* commandJson);
  void processTransponderReleaseConfirm(QJsonObject* commandJson);
  void processTransponderRerelease(QJsonObject* commandJson);
  void processTransponderRereleaseConfirm(QJsonObject* commandJson);

  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createReleaserWaitTimer(void);
  void createGenerator(void);
  void createCommandTemplates(void);

 private slots:
  void on_SocketReadyRead_slot(void);
  void on_SocketDisconnected_slot(void);
  void on_SocketError_slot(QAbstractSocket::SocketError socketError);

  void on_ExpirationTimerTimeout_slot(void);
  void on_DataBlockWaitTimerTimeout_slot(void);
  void on_ReleaserWaitTimerTimeout_slot(void);

 signals:
  void logging(const QString& log) const;
  void disconnected(void);

  void releaserAuthorize_signal(const QMap<QString, QString>* parameters,
                                TransponderReleaseSystem::ReturnStatus* status);
  void releaseRelease_signal(const QMap<QString, QString>* parameters,
                             QMap<QString, QString>* attributes,
                             QMap<QString, QString>* masterKeys,
                             TransponderReleaseSystem::ReturnStatus* status);
  void releaserConfirmRelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* transponderData,
      TransponderReleaseSystem::ReturnStatus* status);
  void releaserRerelease_signal(const QMap<QString, QString>* parameters,
                                QMap<QString, QString>* attributes,
                                QMap<QString, QString>* masterKeys,
                                TransponderReleaseSystem::ReturnStatus* status);
  void releaserConfirmRerelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* transponderData,
      TransponderReleaseSystem::ReturnStatus* status);
  void releaserSearch_signal(const QMap<QString, QString>* parameters,
                             QMap<QString, QString>* attributes,
                             QMap<QString, QString>* masterKeys,
                             TransponderReleaseSystem::ReturnStatus* status);

  void printBoxSticker_signal(
      const QSharedPointer<QMap<QString, QString>> data);
  void printLastBoxSticker_signal(void);
  void printPalletSticker_signal(
      const QSharedPointer<QMap<QString, QString>> data);
  void printLastPalletSticker_signal(void);
};

#endif  // PersoClient_H
