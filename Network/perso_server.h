#ifndef PERSOSERVER_H
#define PERSOSERVER_H

#include <algorithm>

#include <QHash>
#include <QObject>
#include <QSet>
#include <QStack>
#include <QString>
#include <QTcpServer>
#include <QThread>
#include <QTimer>
#include <QtPrintSupport/QPrinterInfo>
#include <QHostAddress>

#include "Management/transponder_release_system.h"
#include "StickerPrinter/isticker_printer.h"
#include "StickerPrinter/te310_printer.h"
#include "perso_client_connection.h"

class PersoServer : public QTcpServer {
  Q_OBJECT
 public:
  enum OperatingState {
    Idle,
    Work,
    Paused,
    Panic,
  };
  Q_ENUM(OperatingState);
  enum ReturnStatus {
    Completed,
    Failed,
    ReleaserError,
  };
  Q_ENUM(ReturnStatus);

 private:
  bool LogEnable;

  int32_t MaxNumberClientConnections;
  int32_t RestartPeriod;
  QHostAddress ListeningAddress;
  uint32_t ListeningPort;
  OperatingState CurrentState;

  QStack<int32_t> FreeClientIds;
  QHash<int32_t, QThread*> ClientThreads;
  QHash<int32_t, PersoClientConnection*> Clients;

  TransponderReleaseSystem* Releaser;
  QThread* ReleaserThread;

  QString BoxStickerPrinterName;
  QString PalletStickerPrinterName;
#ifdef __linux__
  QHostAddress BoxStickerPrinterIP;
  int32_t BoxStickerPrinterPort;
  QHostAddress PalletStickerPrinterIP;
  int32_t PalletStickerPrinterPort;
#endif /* __linux__ */
  IStickerPrinter* BoxStickerPrinter;
  IStickerPrinter* PalletStickerPrinter;

  QTimer* RestartTimer;

 public:
  explicit PersoServer(QObject* parent);
  ~PersoServer();

 public:
  bool start(void);
  void stop(void);

 protected:
  // Внутренний метод вызываемый при получении нового запроса на подключение
  virtual void incomingConnection(qintptr socketDescriptor) override;

 private:
  Q_DISABLE_COPY_MOVE(PersoServer);
  void loadSettings(void);
  void sendLog(const QString& log) const;

  void processCriticalError(const QString& log);
  bool checkConfiguration(void);

  void createReleaserInstance(void);
  void createClientIdentifiers(void);
  void createClientInstance(qintptr socketDescriptor);
  void createStickerPrinters(void);
  void createRestartTimer(void);

 private slots:
  void on_ClientDisconnected_slot(void);
  void on_ClientThreadDeleted_slot(void);

  void printBoxSticker_slot(const QHash<QString, QString>* data,
                            IStickerPrinter::ReturnStatus* status);
  void printLastBoxSticker_slot(IStickerPrinter::ReturnStatus* status);
  void printPalletSticker_slot(const QHash<QString, QString>* data,
                               IStickerPrinter::ReturnStatus* status);
  void printLastPalletSticker_slot(IStickerPrinter::ReturnStatus* status);

  void on_RestartTimerTimeout_slot(void);
  void on_ReleaserFailed_slot(TransponderReleaseSystem::ReturnStatus status);

 signals:
  void logging(const QString& log) const;
  void checkNewClientInstance(void);

  void startReleaser_signal(TransponderReleaseSystem::ReturnStatus* status);
  void stopReleaser_signal(void);
};

#endif  // PERSOSERVER_H
