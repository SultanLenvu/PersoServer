#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

#include <QHostAddress>
#include <QObject>

#include "General/definitions.h"

class UserSettings : public QObject {
  Q_OBJECT

 private:
  bool ServerPersoOption;
  bool ServerCommonKeyGeneration;
  QHostAddress PersoServerIP;
  uint32_t PersoServerPort;
  QString MasterKeyFilePath;

  bool ValidationIndicator;

 public:
  explicit UserSettings(QObject* parent);

  bool serverPersoOption() const;
  void setServerPersonalizationOption(bool newServerPersonalizationOption);

  bool serverCommonKeyGeneration() const;
  void setServerCommonKeyGeneration(bool newServerCommonKeyGeneration);

  const QString persoServerIP() const;
  void setPersoServerIP(const QString& newPersoServerIP);

  uint32_t getPersoServerPort() const;
  void setPersoServerPort(const QString newPersonalizationServerPort);

  const QString masterKeyFilePath() const;
  void setMasterKeyFilePath(const QString& newMasterKeyFilePath);

  bool isValid(void) const;
  void reset(void);
};

#endif  // USER_SETTINGS_H
