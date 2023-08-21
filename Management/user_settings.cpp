#include "user_settings.h"

UserSettings::UserSettings(QObject* parent) : QObject(parent) {
  reset();
}

bool UserSettings::serverPersoOption() const {
  return ServerPersoOption;
}

void UserSettings::setServerPersonalizationOption(
    bool newServerPersonalizationOption) {
  ServerPersoOption = newServerPersonalizationOption;
}

bool UserSettings::serverCommonKeyGeneration() const {
  return ServerCommonKeyGeneration;
}

void UserSettings::setServerCommonKeyGeneration(
    bool newServerCommonKeyGeneration) {
  ServerCommonKeyGeneration = newServerCommonKeyGeneration;
}

const QString UserSettings::persoServerIP() const {
  return PersoHostIP.toString();
}

void UserSettings::setPersoHostIP(const QString& newPersoHostIP) {
  PersoHostIP = QHostAddress(newPersoHostIP);

  if (PersoHostIP.isNull() == true)
    ValidationIndicator = false;
}

uint32_t UserSettings::getPersoHostPort() const {
  return PersoHostPort;
}

void UserSettings::setPersoHostPort(
    const QString newPersonalizationServerPort) {
  PersoHostPort = newPersonalizationServerPort.toInt();

  if ((PersoHostPort <= 0) || (PersoHostPort > IP_PORT_MAX_VALUE))
    ValidationIndicator = false;
}

const QString UserSettings::masterKeyFilePath() const {
  return MasterKeyFilePath;
}

void UserSettings::setMasterKeyFilePath(const QString& newMasterKeyFilePath) {
  MasterKeyFilePath = newMasterKeyFilePath;
}

bool UserSettings::isValid() const {
  return ValidationIndicator;
}

void UserSettings::reset() {
  ServerPersoOption = false;
  ServerCommonKeyGeneration = false;
  PersoHostIP = PERSO_SERVER_DEFAULT_IP;
  PersoHostPort = QString(PERSO_SERVER_DEFAULT_PORT).toInt();
  MasterKeyFilePath = QString();

  ValidationIndicator = true;
}
