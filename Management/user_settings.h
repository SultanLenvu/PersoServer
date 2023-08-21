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
  QHostAddress PersoHostIP;
  uint32_t PersoHostPort;
  QString MasterKeyFilePath;

  bool ValidationIndicator;

 public:
  explicit UserSettings(QObject* parent);

  bool serverPersoOption() const;
  void setServerPersonalizationOption(bool newServerPersonalizationOption);

  bool serverCommonKeyGeneration() const;
  void setServerCommonKeyGeneration(bool newServerCommonKeyGeneration);

  const QString persoServerIP() const;
  void setPersoHostIP(const QString& newPersoHostIP);

  uint32_t getPersoHostPort() const;
  void setPersoHostPort(const QString newPersonalizationServerPort);

  const QString masterKeyFilePath() const;
  void setMasterKeyFilePath(const QString& newMasterKeyFilePath);

  bool isValid(void) const;
  void reset(void);
};

#endif  // USER_SETTINGS_H
