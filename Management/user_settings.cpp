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
  return PersoServerIP.toString();
}

void UserSettings::setPersoServerIP(const QString& newPersoServerIP) {
  PersoServerIP = QHostAddress(newPersoServerIP);

  if (PersoServerIP.isNull() == true)
    ValidationIndicator = false;
}

uint32_t UserSettings::getPersoServerPort() const {
  return PersoServerPort;
}

void UserSettings::setPersoServerPort(
    const QString newPersonalizationServerPort) {
  PersoServerPort = newPersonalizationServerPort.toInt();

  if ((PersoServerPort <= 0) || (PersoServerPort > IP_PORT_MAX_VALUE))
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
  PersoServerIP = PERSO_SERVER_DEFAULT_IP;
  PersoServerPort = QString(PERSO_SERVER_DEFAULT_PORT).toInt();
  MasterKeyFilePath = QString();

  ValidationIndicator = true;
}
