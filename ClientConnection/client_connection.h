#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <QCoreApplication>
#include <QDataStream>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include <QThread>
#include <QTimer>

#include "abstract_client_command.h"
#include "abstract_client_connection.h"

class ClientConnection : public AbstractClientConnection {
  Q_OBJECT

 private:
  int32_t Id;
  quintptr SocketDescriptor;
  std::unique_ptr<QTcpSocket> Socket;

  int32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QJsonObject CommandData;
  QJsonObject ResponseData;
  QHash<QString, std::shared_ptr<AbstractClientCommand>> Commands;

  int32_t UnauthorizedExpirationTime;
  std::unique_ptr<QTimer> ExpirationTimer;
  std::unique_ptr<QTimer> DataBlockWaitTimer;

  std::shared_ptr<ProductionContext> Context;

 public:
  explicit ClientConnection(const QString& name,
                            int32_t id,
                            qintptr socketDescriptor);
  ~ClientConnection();

  // AbstractClientConnection interface
 public:
  virtual void onInstanceThreadStarted(void) override;
  virtual int32_t getId(void) const override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ClientConnection)
  void loadSettings(void);
  void sendLog(const QString& log);

  // Блоки данных
  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  bool processReceivedDataBlock(void);

  // Фабричные методы
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createCommands(void);
  void createContext(void);

 private slots:
  void socketReadyRead_slot(void);
  void socketDisconnected_slot(void);
  void socketError_slot(QAbstractSocket::SocketError socketError);

  void expirationTimerTimeout_slot(void);
  void dataBlockWaitTimerTimeout_slot(void);

  void authorized_slot(void);
  void deauthorized_slot(void);

 signals:
};

#endif  // CLIENT_CONNECTION_H
