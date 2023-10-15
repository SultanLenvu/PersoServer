#include "server_manager.h"

ServerManager::ServerManager(QObject* parent) : QObject(parent) {
  setObjectName("ServerManager");
  loadSettings();

  createLoggerInstance();
}

ServerManager::~ServerManager() {
  LoggerThread->quit();
  LoggerThread->wait();
}

void ServerManager::processCommandArguments(const QStringList* args) {
  if (args->contains("-generate_default_config")) {
    generateDefaultSettings();
  }
  if (args->contains("-s")) {
    if (!checkSettings()) {
      return;
    }
    createServerInstance();
    Server->start();
  }
}

void ServerManager::loadSettings() const {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                     QCoreApplication::applicationDirPath());
}

bool ServerManager::checkSettings() const {
  QSettings settings;
  uint32_t temp = 0;
  QFileInfo info;

  emit logging("Проверка файла конфигурации.");

  info.setFile(QString("%1/%2/%3.ini")
                   .arg(QCoreApplication::applicationDirPath(),
                        QCoreApplication::organizationName(),
                        QCoreApplication::applicationName()));
  if (!info.isFile()) {
    emit logging("Отсутствует файл конфигурации.");
    return false;
  }

  if (settings.value("perso_server/max_number_client_connection").toUInt() ==
      0) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "максимальное количество одновременных клиентских подключений. ");
    return false;
  }

  if (QHostAddress(settings.value("perso_server/listen_ip").toString())
          .isNull()) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "IP-адрес прослушиваемый сервером. ");
    return false;
  }

  temp = settings.value("perso_server/listen_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "порт прослушиваемый сервером. ");
    return false;
  }

  if (settings.value("perso_client/connection_max_duration").toUInt() == 0) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "значение максимальной длительности клиентского "
        "подключения. ");
    return false;
  }

  if (settings.value("log_system/log_file_max_number").toUInt() == 0) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "максимальное количество лог-файлов. ");
    return false;
  }

  if (QHostAddress(settings.value("log_system/udp_destination_ip").toString())
          .isNull()) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "IP-адрес для отправки UDP логов. ");
    return false;
  }

  temp = settings.value("log_system/udp_destination_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "порт для отправки UDP логов. ");
    return false;
  }

  if (QHostAddress(settings.value("postgres_controller/server_ip").toString())
          .isNull()) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "IP-адрес сервера базы данных PostgreSQL. ");
    return false;
  }

  temp = settings.value("postgres_controller/server_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "порт сервера базы данных PostgreSQL. ");
    return false;
  }

  info.setFile(settings.value("firmware_generation_system/firmware_base_path")
                   .toString());
  if (!info.isFile()) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "путь к базовому файлу прошивки транспондера. ");
    return false;
  }

  info.setFile(settings.value("firmware_generation_system/firmware_data_path")
                   .toString());
  if (!info.isFile()) {
    emit logging(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "путь к шаблонному файлу данных транспондера. ");
    return false;
  }

  emit logging("Обработка файла конфигурации успешно завершена.");
  return true;
}

void ServerManager::generateDefaultSettings() const {
  QSettings settings;

  // PersoServer
  settings.setValue("perso_server/max_number_client_connection",
                    CLIENT_MAX_COUNT);
  settings.setValue("perso_server/listen_ip", PERSO_SERVER_DEFAULT_LISTEN_IP);
  settings.setValue("perso_server/listen_port",
                    PERSO_SERVER_DEFAULT_LISTEN_PORT);

  // PersoClient
  settings.setValue("perso_client/connection_max_duration",
                    CLIENT_CONNECTION_MAX_DURATION);

  // LogSystem
  settings.setValue("log_system/global_enable", true);
  settings.setValue("log_system/extended_enable", true);
  settings.setValue("log_system/console_log_enable", true);
  settings.setValue("log_system/file_log_enable", true);
  settings.setValue("log_system/log_file_max_number", 10);
  settings.setValue("log_system/udp_log_enable", true);
  settings.setValue("log_system/udp_destination_ip",
                    DEFAULT_LOG_DESTINATION_IP);
  settings.setValue("log_system/udp_destination_port",
                    DEFAULT_LOG_DESTINATION_PORT);

  // Postgres
  settings.setValue("postgres_controller_controller/server_ip",
                    POSTGRES_DEFAULT_SERVER_IP);
  settings.setValue("postgres_controller_controller/server_port",
                    POSTGRES_DEFAULT_SERVER_PORT);
  settings.setValue("postgres_controller_controller/database_name",
                    POSTGRES_DEFAULT_DATABASE_NAME);
  settings.setValue("postgres_controller_controller/user_name",
                    POSTGRES_DEFAULT_USER_NAME);
  settings.setValue("postgres_controller_controller/user_password",
                    POSTGRES_DEFAULT_USER_PASSWORD);

  // FirmwareGenerationSystem
  settings.setValue("firmware_generation_system/firmware_base_path",
                    DEFAULT_FIRMWARE_BASE_PATH);
  settings.setValue("firmware_generation_system/firmware_data_path",
                    DEFAULT_FIRMWARE_DATA_PATH);
}

void ServerManager::createServerInstance() {
  Server = new PersoServer(this);
  connect(Server, &PersoServer::logging, Logger, &LogSystem::generate);
}

void ServerManager::createLoggerInstance() {
  Logger = LogSystem::instance();
  connect(this, &ServerManager::logging, Logger, &LogSystem::generate);

  LoggerThread = new QThread(this);
  connect(LoggerThread, &QThread::finished, LoggerThread,
          &QThread::deleteLater);

  Logger->moveToThread(LoggerThread);
  LoggerThread->start();
}
