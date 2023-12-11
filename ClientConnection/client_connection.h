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
  size_t Id;
  QTcpSocket* Socket;

  size_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QJsonObject CommandData;
  QJsonObject ResponseData;
  QHash<QString, std::shared_ptr<AbstractClientCommand>> Commands;

  size_t IdleExpirationTime;
  std::unique_ptr<QTimer> ExpirationTimer;
  std::unique_ptr<QTimer> DataBlockWaitTimer;

  bool Authorized;
  QString Login;
  QString Password;

 public:
  explicit ClientConnection(const QString& name,
                            uint32_t id,
                            qintptr socketDescriptor);
  ~ClientConnection();

  // AbstractClientConnection interface
 public:
  virtual size_t getId(void) const override;
  virtual bool isAuthorised() const override;
  virtual const QString& getLogin() const override;
  virtual const QString& getPassword() const override;

 private:
  Q_DISABLE_COPY_MOVE(ClientConnection)
  void loadSettings(void);
  void sendLog(const QString& log) const;

  // Блоки данных
  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  bool processReceivedDataBlock(void);

  // Фабричные методы
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createCommands(void);

 private slots:
  void socketReadyRead_slot(void);
  void socketDisconnected_slot(void);
  void socketError_slot(QAbstractSocket::SocketError socketError);

  void expirationTimerTimeout_slot(void);
  void dataBlockWaitTimerTimeout_slot(void);

  void authorized_slot(const QString& login, const QString& password);
  void deauthorized_slot(void);

 signals:
};

#endif  // CLIENT_CONNECTION_H
