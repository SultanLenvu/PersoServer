#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QFileInfo>
#include <QObject>
#include <QStringList>
#include <QTextCodec>
#include <QThread>

#include "global_environment.h"
#include "log_system.h"
#include "perso_server.h"

class ServerManager : public QObject {
  Q_OBJECT

 private:
  std::unique_ptr<PersoServer> Server;

  std::unique_ptr<LogSystem> Logger;
  GlobalEnvironment* GlobalEnv;

 public:
  explicit ServerManager(const QString& name);
  ~ServerManager();

  bool init(void);

 private:
  ServerManager();
  Q_DISABLE_COPY_MOVE(ServerManager);
  void loadSettings(void) const;

  bool checkSettings(void) const;
  bool checkPersoServerSettings(const QSettings& settings) const;
  bool checkBoxStickerPrinterSettings(const QSettings& settings) const;
  bool checkPalletStickerPrinterSettings(const QSettings& settings) const;
  bool checkClientConnectionSettings(const QSettings& settings) const;
  bool checkLogSystemSettings(const QSettings& settings) const;
  bool checkPostgreSqlDatabaseSettings(const QSettings& settings) const;
  bool checkFirmwareGenerationSystemSettings(const QSettings& settings) const;

  void generateDefaultSettings(void) const;
  void processCommandArguments(void);

  void createServerInstance(void);
  void createLoggerInstance(void);

  void registerMetaType(void);
};

#endif  // SERVERMANAGER_H
