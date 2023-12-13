#ifndef TYPES_H
#define TYPES_H

#include <QHash>

#include "Database/sql_query_values.h"

using StringDictionary = QHash<QString, QString>;
using ProductionLineContext = QHash<QString, std::shared_ptr<SqlQueryValues>>;

enum class ReturnStatus {
  NoError = 0,
  ParameterError,
  SyntaxError,
  SynchronizationError,
  FileOpenError,
  InvalidProductionLineContext,
  UnauthorizedRequest,

  DatabaseConnectionError,
  DatabaseTransactionError,
  DatabaseQueryError,

  FirmwareGeneratorInitError,
  StickerPrinterInitError,

  RecordMissed,
  ProductionLineMissed,
  TranspoderMissed,
  BoxMissed,
  PalletMissed,
  OrderMissed,
  IssuerMissed,
  MasterKeysMissed,

  TransponderNotReleasedEarlier,
  TransponderNotAwaitingConfirmation,
  TransponderIncorrectRerelease,
  IdenticalUcidError,
  CurrentOrderAssembled,

  ProductionLineLaunchError,
  ProductionLineAlreadyLaunched,
  ProductionLineNotActive,
  ProductionLineNotInProcess,
  ProductionLineCompleted,
  ProductionLineShutdownError,
  ProductionLineRollbackLimit,
  OrderInProcessMissed,
  FreeBoxMissed,

  FirmwareGenerationError,

  PrinterConnectionError,
  PrinterLibraryError,
  BoxStickerPrintError,
  PalletStickerPrintError,
  Unknown,
};

#endif  // TYPES_H
