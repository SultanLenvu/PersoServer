#ifndef TYPES_H
#define TYPES_H

#include <QHash>

using StringDictionary = QHash<QString, QString>;
// using ProductionLineContext = QHash<QString,
// std::shared_ptr<SqlQueryValues>>;

template <typename T>
using SharedVector = std::shared_ptr<std::vector<T>>;

enum class ProductionLineState {
  NotActive,
  Idle,
  Launched,
  Completed,
};

enum class ReturnStatus {
  NoError = 0,
  DynamicLibraryMissing,
  ParameterError,
  SyntaxError,
  ConsistencyViolation,
  FileOpenError,

  DatabaseConnectionError,
  DatabaseTransactionError,
  DatabaseQueryError,

  ProductionLineLaunchSystemInitError,
  FirmwareGeneratorInitError,

  RecordMissed,
  ProductionLineMissed,
  TranspoderMissed,
  BoxMissed,
  PalletMissed,
  OrderMissed,
  IssuerMissed,
  MasterKeysMissed,

  OrderMultiplyAssembly,
  OrderInProcessMissed,

  BoxAlreadyRequested,
  BoxNotRequested,
  FreeBoxMissed,
  BoxCompletelyAssembled,
  BoxNotCompletelyAssembled,

  TransponderRepeatRelease,
  TransponderNotReleasedEarlier,
  TransponderNotAwaitingConfirmation,
  TransponderIncorrectRerelease,
  IdenticalUcidError,
  CurrentOrderAssembled,
  TransponderRollbackLimit,

  ProductionLineContextNotAuthorized,
  ProductionLineNotActive,
  ProductionLineNotLaunched,
  ProductionLineNotInProcess,
  ProductionLineLaunchError,
  ProductionLineAlreadyLaunched,
  ProductionLineAlreadyInProcess,
  ProductionLineCompleted,
  ProductionLineShutdownError,

  FirmwareGenerationError,

  StickerPrinterDriverMissed,
  StickerPrinterLibraryMissing,
  StickerPrinterConnectionError,
  BoxStickerPrintError,
  PalletStickerPrintError,
  Unknown,
};

#endif  // TYPES_H
