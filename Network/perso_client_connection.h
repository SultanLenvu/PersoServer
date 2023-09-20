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

class PersoClientConnection : public QObject {
  Q_OBJECT

 private:
  int32_t MaximumConnectionTime;
  uint32_t ID;

  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QJsonDocument CurrentCommand;
  QJsonDocument CurrentResponse;

  QTimer* ExpirationTimer;
  QTimer* WaitTimer;

 public:
  explicit PersoClientConnection(uint32_t id, qintptr socketDescriptor);
  ~PersoClientConnection();

  uint32_t getId(void);
  void applySettings(void);

 public slots:
  void instanceTesting(void);

 private:
  void loadSettings(void);
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createWaitTimer(void);

  void processingDataBlock(void);

  void createDataBlock(void);
  void transmitDataBlock(void);

  void processingEchoRequest(QJsonObject* commandJson);
  void processingFirmwareRequest(QJsonObject* commandJson);

 private slots:
  void proxyLogging(const QString& log);
  void on_SocketReadyRead_slot(void);
  void on_SocketDisconnected_slot(void);
  void on_SocketError_slot(QAbstractSocket::SocketError socketError);

  void on_ExpirationTimerTimeout_slot(void);
  void on_WaitTimerTimeout_slot(void);

 signals:
  void disconnected(void);

  void logging(const QString& log);
};

#endif  // PERSOCLIENTCONNECTION_H
