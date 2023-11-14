#include <cstdlib>

#include "server_manager.h"

ServerManager::ServerManager(QObject* parent) : QObject(parent) {
  setObjectName("ServerManager");
  loadSettings();
  registerMetaType();
}

ServerManager::~ServerManager() {
  LoggerThread->quit();
  LoggerThread->wait();
}

void ServerManager::processCommandArguments(const QStringList* args) {
  if (args->contains("-generate_default_config")) {
    generateDefaultSettings();
    exit(0);
  }
}

void ServerManager::loadSettings() const {
  QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(PROGRAM_NAME);

  QSettings::setDefaultFormat(QSettings::IniFormat);
}

bool ServerManager::checkSettings() const {
  QSettings settings;
  uint32_t temp = 0;
  QFileInfo info(settings.fileName());

  /* Logger is not here yet, so we have to use stdout and stderr */
  qDebug("Проверка файла настроек.");

  info.setFile(settings.fileName());
  if (!info.isFile()) {
    qCritical("Отсутствует файл конфигурации.");
    return false;
  }

  if (settings.value("perso_server/max_number_client_connection").toUInt() ==
      0) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "максимальное количество одновременных клиентских подключений. ");
    return false;
  }

  if (settings.value("perso_server/restart_period").toUInt() == 0) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "зачение периода для попытки перезапуска сервера. ");
    return false;
  }

  if (QHostAddress(settings.value("perso_server/listen_ip").toString())
          .isNull()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "IP-адрес прослушиваемый сервером. ");
    return false;
  }

  temp = settings.value("perso_server/listen_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "порт прослушиваемый сервером. ");
    return false;
  }

#ifdef __linux__
    if (!settings.contains("perso_server/box_sticker_printer_ip")
        || !settings.contains("perso_server/box_sticker_printer_port")) {
      qCritical("Получена ошибка при обработке файла конфигурации: "
          "Не указаны имя или IP-адрес принтера стикеров на боксы");
      return false;
    }
    QHostAddress boxIP(
        settings.value("perso_server/box_sticker_printer_ip").toString());
    int boxPort = settings.value(
        "perso_server/box_sticker_printer_port").toInt();

    if (boxIP.isNull() || boxPort <= 0 || boxPort > 65535) {
      qCritical("Получена ошибка при обработке файла конфигурации: "
          "неверный IP-адрес или порт принтера стикеров на боксы");
      return false;
    }

    if (!settings.contains("perso_server/pallet_sticker_printer_ip")
        || !settings.contains("perso_server/pallet_sticker_printer_port")) {
      qCritical("Получена ошибка при обработке файла конфигурации: "
                "Не указаны имя или IP-адрес принтера стикеров на паллеты.");
      return false;
    }
    QHostAddress palletIP(settings.value("perso_server/pallet_sticker_printer_ip").toString());
    int palletPort = settings.value("perso_server/pallet_sticker_printer_port").toInt();

    if (palletIP.isNull() || palletPort <= 0 || palletPort > 6553500) {
      qCritical("Получена ошибка при обработке файла конфигурации: "
                "неверный IP-адрес или порт принтера стикеров на паллеты.");
      return false;
    }
#else
  if (settings.value("perso_server/box_sticker_printer_name")
          .toString()
          .isEmpty()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: не указано имя "
        "принтера для печати стикеров на боксы. ");
    return false;
  }

  if (settings.value("perso_server/pallet_sticker_printer_name")
          .toString()
          .isEmpty()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: не указано имя "
        "принтера для печати стикеров на паллеты. ");
    return false;
  }
#endif /* __linux__ */
  if (settings.value("transponder_release_system/check_period").toUInt() == 0) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "зачение периода проверки системы выпуска транспондеров. ");
    return false;
  }

  if (settings.value("perso_client/connection_max_duration").toUInt() == 0) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "значение максимальной длительности клиентского "
        "подключения. ");
    return false;
  }

  if (settings.value("log_system/log_file_max_number").toUInt() == 0) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректное "
        "максимальное количество лог-файлов. ");
    return false;
  }

  if (QHostAddress(settings.value("log_system/udp_destination_ip").toString())
          .isNull()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "IP-адрес для отправки UDP логов. ");
    return false;
  }

  temp = settings.value("log_system/udp_destination_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "порт для отправки UDP логов. ");
    return false;
  }

  if (QHostAddress(settings.value("postgres_controller/server_ip").toString())
          .isNull()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "IP-адрес сервера базы данных PostgreSQL. ");
    return false;
  }

  temp = settings.value("postgres_controller/server_port").toUInt();
  if ((temp <= IP_PORT_MIN_VALUE) || (temp > IP_PORT_MAX_VALUE)) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "порт сервера базы данных PostgreSQL. ");
    return false;
  }

  info.setFile(settings.value("firmware_generation_system/firmware_base_path")
                   .toString());
  if (!info.isFile()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "путь к базовому файлу прошивки транспондера. ");
    return false;
  }

  info.setFile(settings.value("firmware_generation_system/firmware_data_path")
                   .toString());
  if (!info.isFile()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "путь к шаблонному файлу данных транспондера. ");
    return false;
  }

  info.setFile(settings.value("te310_printer/library_path").toString());
  if (!info.isFile()) {
    qCritical(
        "Получена ошибка при обработке файла конфигурации: некорректный "
        "путь к библиотеке принтера TSC TE310. ");
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

  // PersoClientConnection
  settings.setValue("perso_client/connection_max_duration",
                    CLIENT_CONNECTION_MAX_DURATION);

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
  settings.setValue("postgres_controller/server_ip",
                    POSTGRES_DEFAULT_SERVER_IP);
  settings.setValue("postgres_controller/server_port",
                    POSTGRES_DEFAULT_SERVER_PORT);
  settings.setValue("postgres_controller/database_name",
                    POSTGRES_DEFAULT_DATABASE_NAME);
  settings.setValue("postgres_controller/user_name",
                    POSTGRES_DEFAULT_USER_NAME);
  settings.setValue("postgres_controller/user_password",
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

void ServerManager::start() {
  createLoggerInstance();
  createServerInstance();
  Server->start();
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

void ServerManager::registerMetaType() {
  qRegisterMetaType<QSharedPointer<QHash<QString, QString>>>(
      "QSharedPointer<QHash<QString, QString> >");
  qRegisterMetaType<QSharedPointer<QHash<QString, QString>>>(
      "QSharedPointer<QHash<QString, QString> >");
  qRegisterMetaType<QSharedPointer<QStringList>>("QSharedPointer<QStringList>");
  qRegisterMetaType<QSharedPointer<QFile>>("QSharedPointer<QFile>");
  qRegisterMetaType<TransponderReleaseSystem::ReturnStatus>(
      "TransponderReleaseSystem::ReturnStatus");
}
