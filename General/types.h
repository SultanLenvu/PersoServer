#ifndef TYPES_H
#define TYPES_H

#include <QHash>

using StringDictionary = QHash<QString, QString>;

enum class ReturnStatus {
  NoError = 0,
  ArgumentError,
  SyntaxError,
  DatabaseError,
  FirmwareGenerationError,
  TransponderNotFound,
  FreeBoxMissed,
  TransponderNotReleasedEarlier,
  AwaitingConfirmationError,
  IdenticalUcidError,
  ProductionLineMissed,
  ProductionLineNotActive,
  CurrentOrderRunOut,
  CurrentOrderAssembled,
  ProductionLineRollbackLimitError,
  BoxStickerPrintError,
  PalletStickerPrintError,
  NextTransponderNotFound,
  StartBoxAssemblingError,
  StartPalletAssemblingError,
  Unknown,
};

#endif  // TYPES_H
