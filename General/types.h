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
  InvalidProductionContext,
  UnauthorizedRequest,

  DatabaseConnectionError,
  DatabaseTransactionError,
  DatabaseQueryError,

  FirmwareGeneratorInitError,
  StickerPrinterInitError,

  ProductionLineMissed,
  TranspoderMissed,
  BoxMissed,
  PalletMissed,
  OrderMissed,
  IssuerMissed,
  MasterKeysMissed,

  TransponderWrongRerelease,
  TransponderNotReleasedEarlier,
  TransponderNotAwaitingConfirmation,
  IdenticalUcidError,
  CurrentOrderAssembled,
  NextTransponderNotFound,

  ProductionLineLaunchError,
  ProductionLineAlreadyLaunched,
  ProductionLineNotActive,
  ProductionLineNotInProcess,
  OrderInProcessMissed,
  FreeBoxMissed,
  ProductionLineShutdownError,
  ProductionLineRollbackLimit,

  FirmwareGenerationError,

  BoxStickerPrintError,
  PalletStickerPrintError,
  Unknown,
};

#endif  // TYPES_H
