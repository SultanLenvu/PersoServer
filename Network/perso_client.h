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

  QMap<QString, void (PersoClient::*)(void) const> CommandHandlers;
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

  // Блоки данных
  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  void processReceivedDataBlock(void);

  // Обработчики команд
  void processSyntaxError(void) const;
  void processEcho(void) const;
  void processAuthorization(void) const;

  void processTransponderRelease(void) const;
  void processTransponderReleaseConfirm(void) const;
  void processTransponderRerelease(void) const;
  void processTransponderRereleaseConfirm(void) const;

  void processPrintBoxSticker(void) const;
  void processPrintLastBoxSticker(void) const;
  void processPrintPalletSticker(void) const;
  void processPrintLastPalletSticker(void) const;

  // Фабричные методы
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createReleaserWaitTimer(void);
  void createGenerator(void);
  void createCommandHandlers(void);
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
  void disconnected(void) const;

  void releaserAuthorize_signal(
      const QMap<QString, QString>* parameters,
      TransponderReleaseSystem::ReturnStatus* status) const;
  void releaseRelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* attributes,
      QMap<QString, QString>* masterKeys,
      TransponderReleaseSystem::ReturnStatus* status) const;
  void releaserConfirmRelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* transponderData,
      TransponderReleaseSystem::ReturnStatus* status) const;
  void releaserRerelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* attributes,
      QMap<QString, QString>* masterKeys,
      TransponderReleaseSystem::ReturnStatus* status) const;
  void releaserConfirmRerelease_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* transponderData,
      TransponderReleaseSystem::ReturnStatus* status) const;
  void releaserSearch_signal(
      const QMap<QString, QString>* parameters,
      QMap<QString, QString>* attributes,
      QMap<QString, QString>* masterKeys,
      TransponderReleaseSystem::ReturnStatus* status) const;

  void printBoxSticker_signal(
      const QSharedPointer<QMap<QString, QString>> data) const;
  void printLastBoxSticker_signal(void) const;
  void printPalletSticker_signal(
      const QSharedPointer<QMap<QString, QString>> data);
  void printLastPalletSticker_signal(void) const;
};

#endif  // PersoClient_H
