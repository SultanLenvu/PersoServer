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

#include "Database/database_controller_interface.h"
#include "Database/postgres_controller.h"
#include "user_settings.h"

class PersoClientConnection : public QObject {
  Q_OBJECT

 private:
  uint32_t ID;
  qintptr SocketDescriptor;
  QSettings* Settings;

  QTcpSocket* Socket;

  QByteArray ReceivedRawData;
  QJsonDocument CurrentCommand;
  QJsonDocument CurrentResponse;

  QTimer* ExpirationTimer;

 public:
  explicit PersoClientConnection(uint32_t id,
                                 qintptr socketDescriptor,
                                 QSettings* settings);
  ~PersoClientConnection();

  uint32_t getId(void);

 public slots:
  void instanceTesting(void);

 private:
  void processingReceivedRawData(void);
  void transmitResponseRawData(void);

  void echoRequestProcessing(QJsonObject* json);
  void getFirmwareProcessing(QJsonObject* json);

 private slots:
  void proxyLogging(const QString& log);
  void on_SocketReadyRead_slot(void);
  void on_SocketDisconnected_slot(void);

  void on_ExpirationTimerTimeout_slot(void);

 signals:
  void disconnected(void);

  void logging(const QString& log);
};

#endif  // PERSOCLIENTCONNECTION_H
