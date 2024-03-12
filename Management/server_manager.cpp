#include <QCoreApplication>
#include <QLibrary>

#include "definitions.h"
#include "server_manager.h"
#include "types.h"

ServerManager::ServerManager(const QString& name) : QObject(nullptr) {
  setObjectName(name);
  loadSettings();
  registerMetaType();
}

ServerManager::~ServerManager() {}

bool ServerManager::init() {
  processCommandArguments();

  if (!checkSettings()) {
    return false;
  }

  Logger = std::unique_ptr<LogSystem>(new LogSystem("LogSystem"));
  GlobalEnv = GlobalEnvironment::instance();

  createServerInstance();
  if (!Server->start()) {
    qCritical("Не удалось запустить сервер.");
  }

  return true;
}

void ServerManager::loadSettings() const {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);
}

bool ServerManager::checkSettings() const {
  QSettings settings;
  QFileInfo info(settings.fileName());

  /* Logger is not here yet, so we have to use stdout and stderr */
  qDebug("Проверка файла настроек.");

  info.setFile(settings.fileName());
  if (!info.isFile()) {
    qCritical("Отсутствует файл конфигурации.");
    return false;
  }

  if (!checkPersoServerSettings(settings) ||
      !checkBoxStickerPrinterSettings(settings) ||
      !checkPalletStickerPrinterSettings(settings) ||
      !checkClientConnectionSettings(settings) ||
      !checkLogSystemSettings(settings) ||
      !checkPostgreSqlDatabaseSettings(settings) ||
      !checkFirmwareGenerationSystemSettings(settings)) {
    qCritical("Получена ошибка при проверке файла конфигурации.");
    return false;
  }

  return true;
}

bool ServerManager::checkPersoServerSettings(const QSettings& settings) const {
  uint32_t temp = 0;

  if (settings.value("perso_server/max_number_client_connection").toUInt() ==
      0) {
    qCritical(
        "Некорректное максимальное количество одновременных клиентских "
        "подключений. ");
    return false;
  }

  if (settings.value("perso_server/restart_period").toUInt() == 0) {
    qCritical("Некорректное зачение периода для попытки перезапуска сервера. ");
    return false;
  }

  if (QHostAddress(settings.value("perso_server/listen_ip").toString())
          .isNull()) {
    qCritical("Некорректный IP-адрес прослушиваемый сервером. ");
    return false;
  }

  temp = settings.value("perso_server/listen_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical("Некорректный порт прослушиваемый сервером. ");
    return false;
  }

  return true;
}

bool ServerManager::checkBoxStickerPrinterSettings(
    const QSettings& settings) const {
  uint32_t temp = 0;

  if (!QLibrary::isLibrary(
          settings.value("box_sticker_printer/library_path").toString())) {
    qCritical("Некоректный файл библиотеки для принтера стикеров. ");
    return false;
  }

  if (QHostAddress(settings.value("box_sticker_printer/ip").toString())
          .isNull()) {
    qCritical("Некорректный IP-адрес принтера стикеров для боксов. ");
    return false;
  }

  temp = settings.value("box_sticker_printer/port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical("Некорректный IP-адрес принтера стикеров для боксов. ");
    return false;
  }

  return true;
}

bool ServerManager::checkPalletStickerPrinterSettings(
    const QSettings& settings) const {
  uint32_t temp = 0;

  if (!QLibrary::isLibrary(
          settings.value("pallet_sticker_printer/library_path").toString())) {
    qCritical("Некоректный файл библиотеки для принтера стикеров. ");
    return false;
  }

  if (QHostAddress(settings.value("pallet_sticker_printer/ip").toString())
          .isNull()) {
    qCritical("Некорректный IP-адрес принтера стикеров для боксов. ");
    return false;
  }

  temp = settings.value("pallet_sticker_printer/port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical("Некорректный IP-адрес принтера стикеров для боксов. ");
    return false;
  }

  return true;
}

bool ServerManager::checkClientConnectionSettings(
    const QSettings& settings) const {
  if (settings.value("client_connection/unauthorized_access_expiration_time")
          .toUInt() == 0) {
    qCritical(
        "Некорректное значение максимальной длительности неавторизированного "
        "клиентского подключения. ");
    return false;
  }

  return true;
}

bool ServerManager::checkLogSystemSettings(const QSettings& settings) const {
  uint32_t temp = 0;
  QFileInfo info;

  if (settings.value("log_system/message_max_size").toUInt() == 0) {
    qCritical("Некорректный максимальный размер сообщений лога. ");
    return false;
  }

  info.setFile(settings.value("log_system/file_directory").toString());
  if (!info.isDir()) {
    qCritical("Некорректный путь для сохранения лог-файлов. ");
    return false;
  }

  if (QHostAddress(settings.value("log_system/udp_destination_ip").toString())
          .isNull()) {
    qCritical("Некорректный IP-адрес для отправки UDP логов. ");
    return false;
  }

  temp = settings.value("log_system/udp_destination_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical("Некорректный порт для отправки UDP логов. ");
    return false;
  }

  return true;
}

bool ServerManager::checkPostgreSqlDatabaseSettings(
    const QSettings& settings) const {
  uint32_t temp = 0;

  if (QHostAddress(settings.value("postgre_sql_database/server_ip").toString())
          .isNull()) {
    qCritical("Некорректный IP-адрес сервера базы данных PostgreSQL. ");
    return false;
  }

  temp = settings.value("postgre_sql_database/server_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical("Некорректный порт сервера базы данных PostgreSQL. ");
    return false;
  }

  return true;
}

bool ServerManager::checkFirmwareGenerationSystemSettings(
    const QSettings& settings) const {
  QFileInfo info;

  info.setFile(settings.value("firmware_generation_system/firmware_base_path")
                   .toString());
  if (!info.isFile()) {
    qCritical("Некорректный путь к базовому файлу прошивки транспондера. ");
    return false;
  }

  info.setFile(settings.value("firmware_generation_system/firmware_data_path")
                   .toString());
  if (!info.isFile()) {
    qCritical("Некорректный путь к шаблонному файлу данных транспондера. ");
    return false;
  }

  return true;
}

void ServerManager::generateDefaultSettings() const {
  QSettings settings;

  // PersoServer
  settings.setValue("perso_server/restart_period", RESTART_DEFAULT_PERIOD);
  settings.setValue("perso_server/max_number_client_connection",
                    CLIENT_MAX_COUNT);
  settings.setValue("perso_server/listen_ip", PERSO_SERVER_DEFAULT_LISTEN_IP);
  settings.setValue("perso_server/listen_port",
                    PERSO_SERVER_DEFAULT_LISTEN_PORT);
  settings.setValue("perso_server/box_sticker_printer_name",
                    PRINTER_FOR_BOX_DEFAULT_NAME);
  settings.setValue("perso_server/pallet_sticker_printer_name",
                    PRINTER_FOR_PALLET_DEFAULT_NAME);

  // ClientConnection
  settings.setValue("perso_client/unauthorized_access_expiration_time",
                    CLIENT_IDLE_EXPIRATION_TIME);

  // LogSystem
  settings.setValue("log_system/global_enable", true);
  settings.setValue("log_system/extended_enable", true);
  settings.setValue("log_system/console_log_enable", true);
  settings.setValue("log_system/file_log_enable", true);
  settings.setValue("log_system/log_file_max_number",
                    LOG_FILE_DEFAULT_MAX_NUMBER);
  settings.setValue("log_system/udp_log_enable", true);
  settings.setValue("log_system/udp_destination_ip",
                    UDP_LOG_DESTINATION_DEFAULT_IP);
  settings.setValue("log_system/udp_destination_port",
                    UDP_LOG_DESTINATION_DEFAULT_PORT);

  // Postgres
  settings.setValue("postgre_sql_database/server_ip",
                    POSTGRES_DEFAULT_SERVER_IP);
  settings.setValue("postgre_sql_database/server_port",
                    POSTGRES_DEFAULT_SERVER_PORT);
  settings.setValue("postgre_sql_database/database_name",
                    POSTGRES_DEFAULT_DATABASE_NAME);
  settings.setValue("postgre_sql_database/user_name",
                    POSTGRES_DEFAULT_USER_NAME);
  settings.setValue("postgre_sql_database/user_password",
                    POSTGRES_DEFAULT_USER_PASSWORD);

  // TransponderReleaseSystem
  settings.setValue("transponder_release_system/check_period",
                    TRS_DEFAULT_CHECK_PERIOD);

  // FirmwareGenerationSystem
  settings.setValue("firmware_generation_system/firmware_base_path",
                    DEFAULT_FIRMWARE_BASE_PATH);
  settings.setValue("firmware_generation_system/firmware_data_path",
                    DEFAULT_FIRMWARE_DATA_PATH);

  // TE310Printer
  settings.setValue("te310_printer/library_path",
                    TSC_TE310_LIBRARY_DEFAULT_PATH);
}

void ServerManager::processCommandArguments() {
  QStringList args = QCoreApplication::arguments();

  if (args.contains("-generate_default_config")) {
    generateDefaultSettings();
    QCoreApplication::exit(0);
  }
}

void ServerManager::createServerInstance() {
  Server = std::unique_ptr<PersoServer>(new PersoServer("PersoServer"));
}

void ServerManager::createLoggerInstance() {}

void ServerManager::registerMetaType() {
  qRegisterMetaType<QSharedPointer<StringDictionary>>(
      "QSharedPointer<StringDictionary >");
  qRegisterMetaType<QSharedPointer<StringDictionary>>(
      "QSharedPointer<StringDictionary >");
  qRegisterMetaType<QSharedPointer<QStringList>>("QSharedPointer<QStringList>");
  qRegisterMetaType<QSharedPointer<QFile>>("QSharedPointer<QFile>");
  qRegisterMetaType<ReturnStatus>("ReturnStatus");
  qRegisterMetaType<StringDictionary>("StringDictionary");
}
