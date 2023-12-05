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
#include "abstract_firmware_generation_system.h"

class ClientConnection : public AbstractClientConnection {
  Q_OBJECT

 private:
  bool LogEnable;
  bool ExtendedLogEnable;

  size_t Id;
  QTcpSocket* Socket;

  uint32_t ReceivedDataBlockSize;
  QByteArray ReceivedDataBlock;
  QByteArray TransmittedDataBlock;

  QHash<QString, std::unique_ptr<AbstractClientCommand>> Commands;

  int32_t MaximumConnectionTime;
  std::unique_ptr<QTimer> ExpirationTimer;
  std::unique_ptr<QTimer> DataBlockWaitTimer;

  std::shared_ptr<AbstractFirmwareGenerationSystem> Generator;

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
  void sendLog(const QString& log);

  // Блоки данных
  void createTransmittedDataBlock(void);
  void transmitDataBlock(void);
  void processReceivedDataBlock(void);

  // Фабричные методы
  void createSocket(qintptr socketDescriptor);
  void createExpirationTimer(void);
  void createDataBlockWaitTimer(void);
  void createGenerator(void);
  void createCommands(void);

 private slots:
  void socketReadyRead_slot(void);
  void socketDisconnected_slot(void);
  void socketError_slot(QAbstractSocket::SocketError socketError);

  void expirationTimerTimeout_slot(void);
  void dataBlockWaitTimerTimeout_slot(void);

 signals:
  void authorize_signal(const StringDictionary& parameters,
                        ReturnStatus& status);
  void release_signal(const StringDictionary& parameters,
                      StringDictionary& seed,
                      StringDictionary& data,
                      ReturnStatus& status);
  void confirmRelease_signal(const StringDictionary& parameters,
                             ReturnStatus& status);
  void rerelease_signal(const StringDictionary& parameters,
                        StringDictionary& seed,
                        StringDictionary& data,
                        ReturnStatus& status);
  void confirmRerelease_signal(const StringDictionary& parameters,
                               ReturnStatus& status);
};

#endif  // CLIENT_CONNECTION_H
