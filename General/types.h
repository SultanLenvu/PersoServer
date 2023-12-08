#ifndef TYPES_H
#define TYPES_H

#include <QHash>

#include "Database/sql_query_values.h"

using StringDictionary = QHash<QString, QString>;
using ProductionContext = QHash<QString, std::shared_ptr<SqlQueryValues>>;

enum class ReturnStatus {
  NoError = 0,
  ArgumentError,
  SyntaxError,
  SynchronizationError,
  FileOpenError,
  ContextError,

  DatabaseConnectionError,
  DatabaseTransactionError,
  DatabaseQueryError,

  ProductionLineMissed,
  TranspoderMissed,
  BoxMissed,
  PalletMissed,
  OrderMissed,
  IssuerMissed,
  MasterKeysMissed,

  TransponderWasReleasedEarlier,
  TransponderAwaitingConfirmationError,

  ProductionLineLaunchError,
  ProductionLineAlreadyLaunched,
  ProductionLineNotActive,
  OrderInProcessMissed,
  FreeBoxMissed,
  ProductionLineShutdownError,

  FirmwareGenerationError,

  TransponderRollbackLimitError,
  AwaitingConfirmationError,
  IdenticalUcidError,
  CurrentOrderRunOut,
  CurrentOrderAssembled,
  BoxStickerPrintError,
  PalletStickerPrintError,
  NextTransponderNotFound,
  StartBoxAssemblingError,
  StartPalletAssemblingError,
  Unknown,
};

#endif  // TYPES_H
