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

  FirmwareGeneratorInitError,
  FirmwareGenerationError,

  RecordMissed,
  ProductionLineMissed,
  TransponderMissed,
  BoxMissed,
  PalletMissed,
  OrderMissed,
  IssuerMissed,
  MasterKeysMissed,

  OrderMultiplyAssembly,
  OrderInProcessMissed,
  OrderCompletelyAssembled,

  BoxAlreadyRequested,
  BoxNotRequested,
  FreeBoxMissed,
  BoxIsEmty,
  BoxOverflow,
  BoxCompletelyAssembled,
  BoxNotCompletelyAssembled,

  PalletIsEmpty,
  PalletOverflow,

  TransponderRepeatRelease,
  TransponderNotReleasedEarlier,
  TransponderNotAwaitingConfirmation,
  TransponderIncorrectRerelease,
  TransponderIdenticalUcidError,
  TransponderRollbackLimit,

  ProductionLineLaunchSystemInitError,
  ProductionLineContextNotAuthorized,
  ProductionLineNotActive,
  ProductionLineNotLaunched,
  ProductionLineNotInProcess,
  ProductionLineLaunchError,
  ProductionLineAlreadyLaunched,
  ProductionLineAlreadyInProcess,
  ProductionLineCompleted,
  ProductionLineShutdownError,

  StickerPrinterDriverMissed,
  StickerPrinterLibraryMissing,
  StickerPrinterConnectionError,
  StickerPrintError,
  LastStickerMissed,

  Unknown,
};

#endif  // TYPES_H
