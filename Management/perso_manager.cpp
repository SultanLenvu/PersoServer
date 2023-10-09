#include "perso_manager.h"

PersoManager::PersoManager(QObject *parent) : QObject(parent)
{
  setObjectName("PersoManager");
  loadSettings();

  createLoggerInstance();
  createServerInstance();
}

PersoManager::~PersoManager() {
  LoggerThread->quit();
  LoggerThread->wait();
}

void PersoManager::processCommandArguments(const QStringList* args) {
  if (args->contains("-s")) {
    Server->start();
  }
}

void PersoManager::loadSettings() {
  std::cout << "Загрузка конфигурации" << std::endl;
  QSettings settings;

  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                     QCoreApplication::applicationDirPath());

  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);

  std::cout << settings.value("test_setting_1").toString().toUtf8().constData();
  settings.setValue("test_setting_2", "value");
}

void PersoManager::createServerInstance() {
  Server = new PersoServer(this);
  connect(Server, &PersoServer::logging, Logger, &LogSystem::generate);
}

void PersoManager::createLoggerInstance() {
  Logger = new LogSystem(nullptr);

  LoggerThread = new QThread(this);
  connect(LoggerThread, &QThread::finished, LoggerThread,
          &QThread::deleteLater);
  connect(LoggerThread, &QThread::finished, Logger, &LogSystem::deleteLater);

  Logger->moveToThread(LoggerThread);
  LoggerThread->start();
}
