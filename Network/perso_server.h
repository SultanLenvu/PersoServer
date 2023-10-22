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

#include "Log/log_system.h"
#include "Management/transponder_release_system.h"
#include "Management/transponder_seed.h"
#include "StickerPrinter/isticker_printer.h"
#include "StickerPrinter/te310_printer.h"
#include "perso_client.h"

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
  QHash<int32_t, PersoClient*> Clients;

  TransponderReleaseSystem* Releaser;
  QThread* ReleaserThread;

  QString PrinterForBoxSticker;
  QString PrinterForPalletSticker;
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
  Q_DISABLE_COPY(PersoServer);
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

  void printBoxSticker_slot(const QSharedPointer<QHash<QString, QString> > data);
  void printLastBoxSticker_slot(void);
  void printPalletSticker_slot(
      const QSharedPointer<QHash<QString, QString> > data);
  void printLastPalletSticker_slot(void);

  void on_RestartTimerTimeout_slot(void);
  void on_ReleaserFailed_slot(TransponderReleaseSystem::ReturnStatus status);

 signals:
  void logging(const QString& log) const;
  void checkNewClientInstance(void);

  void startReleaser_signal(TransponderReleaseSystem::ReturnStatus* status);
  void stopReleaser_signal(void);
};

#endif  // PERSOSERVER_H