#ifndef FIRMWAREGENERATIONSYSTEM_H
#define FIRMWAREGENERATIONSYSTEM_H

#include <QAbstractTableModel>
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QObject>
#include <QSettings>
#include <QString>

#include "abstract_firmware_generation_system.h"

/*
// Физическое представление данных транспондера: данные, не изменяемые
транзакциях typedef struct
{
        // Атрибуты приложения EFC и системного приложения
DsrcAttribute efcAttributes[CONST_EFC_ATTRIBUTES_COUNT];
DsrcAttribute systemAttributes[CONST_SYSTEM_ATTRIBUTES_COUNT];

// Ключи безопасности приложения EFC
SecurityKey keys[SECURITY_KEYS_COUNT];  // Ключи безопасности транзакции

// Системный пароль
uint8_t password[SYSTEM_PASSWORD_LENGTH];

// Ключ инициализации
uint32_t initKey;
}
ConstObuDataType;

// Физическое представление данных транспондера: данные, изменяемые в
// транзакциях
typedef struct {
  // Атрибуты приложения EFC и системного приложения
  DsrcAttribute efcAttributes[VAR_EFC_ATTRIBUTES_COUNT];
  DsrcAttribute systemAttributes[VAR_SYSTEM_ATTRIBUTES_COUNT];

  // Данные о предыдущей транзакции
  PreviousTransactionData previousTransaction;

  // Логирование для тестирования и сбора статистики
  ObuLog log;

} VarObuDataType;
*/

class FirmwareGenerationSystem : public AbstractFirmwareGenerationSystem {
  Q_OBJECT
 private:
  std::unique_ptr<QFile> FirmwareBaseFile;
  std::unique_ptr<QFile> FirmwareDataFile;

  QHash<QString, QByteArray> CommonKeys;

 public:
  explicit FirmwareGenerationSystem(const QString& name);
  ~FirmwareGenerationSystem();

  virtual bool init(void) override;
  virtual ReturnStatus generate(const StringDictionary& seed,
                                QByteArray& assembledFirmware) override;

 private:
  FirmwareGenerationSystem();
  Q_DISABLE_COPY_MOVE(FirmwareGenerationSystem);
  void loadSettings(void);
  void sendLog(const QString& log) const;
  void createPositionMap(void);

  ReturnStatus assembleFirmware(const QByteArray& firmwareData,
                                QByteArray& assembledFirmware);

  ReturnStatus generateFirmwareData(const StringDictionary& seed,
                                    QByteArray& firmwareData);
  void generateCommonKeys(const StringDictionary& seed);
  void generatePaymentMeans(const QString& pan, QString& paymentMeans);

 signals:
};

#endif  // FIRMWAREGENERATIONSYSTEM_H
