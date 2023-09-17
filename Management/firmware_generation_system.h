#ifndef FIRMWAREGENERATIONSYSTEM_H
#define FIRMWAREGENERATIONSYSTEM_H

#include <QAbstractTableModel>
#include <QDataStream>
#include <QFile>
#include <QObject>
#include <QSettings>
#include <QString>

#include "General/definitions.h"
#include "Security/des.h"
#include "transponder_seed_model.h"
/*
// Физическое представление данных транспондера: данные, не изменяемые
транзакциях typedef struct
{
  // Атрибуты приложения EFC и системного приложения
  DsrcAttribute efcAttributes[CONST_EFC_ATTRIBUTES_COUNT];
  DsrcAttribute systemAttributes[CONST_SYSTEM_ATTRIBUTES_COUNT];

  // Ключи безопасности приложения EFC
  SecurityKey keys[SECURITY_KEYS_COUNT]; // Ключи безопасности транзакции

  // Системный пароль
  uint8_t password[SYSTEM_PASSWORD_LENGTH];

  // Ключ инициализации
  uint32_t initKey;
} ConstObuDataType;

// Физическое представление данных транспондера: данные, изменяемые в
транзакциях typedef struct
{
  // Атрибуты приложения EFC и системного приложения
  DsrcAttribute efcAttributes[VAR_EFC_ATTRIBUTES_COUNT];
  DsrcAttribute systemAttributes[VAR_SYSTEM_ATTRIBUTES_COUNT];

  // Данные о предыдущей транзакции
  PreviousTransactionData previousTransaction;

  // Логирование для тестирования и сбора статистики
  ObuLog log;

} VarObuDataType;
*/

class FirmwareGenerationSystem : public QObject
{
  Q_OBJECT
 public:
  enum ExecutionStatus {
    NotExecuted,
    GenerationError,
    DatabaseQueryError,
    UnknowError,
    CompletedSuccessfully
  };

 private:
  QFile* FirmwareBase;
  QFile* FirmwareData;

  QByteArray* GeneratedFirmware;

 public:
  explicit FirmwareGenerationSystem(QObject* parent);
  void applySettings(void);

  bool generate(TransponderSeedModel* seed, QByteArray* firmware);

 private:
  void loadSettings(void);

  void generateFirmwareData(void);
  void assembleFirmware(void);

  void generateCommonKeys(TransponderSeedModel* seed);

 signals:
  void logging(const QString& log) const;
};

#endif // FIRMWAREGENERATIONSYSTEM_H
