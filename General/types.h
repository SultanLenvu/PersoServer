#ifndef TYPES_H
#define TYPES_H

#include <QHash>

using StringDictionary = QHash<QString, QString>;
// using ProductionLineContext = QHash<QString,
// std::shared_ptr<SqlQueryValues>>;

template <typename T>
using SharedVector = std::shared_ptr<QVector<T>>;

enum class ProductionLineState {
  NotActive,
  Idle,
  Launched,
  Completed,
};

enum class ReturnStatus {
  NoError = 0,
  ParameterError,
  SyntaxError,
  SynchronizationError,
  FileOpenError,

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

  OrderMultiplyAssembly,
  OrderAssemblyMissing,

  BoxCompletelyAssembled,
  BoxNotCompletelyAssembled,

  TransponderNotReleasedEarlier,
  TransponderNotAwaitingConfirmation,
  TransponderIncorrectRerelease,
  IdenticalUcidError,
  CurrentOrderAssembled,

  ProductionContextNotAuthorized,
  ProductionLineNotLaunched,
  ProductionLineLaunchError,
  ProductionLineAlreadyLaunched,
  ProductionLineAlreadyInProcess,
  ProductionLineNotActive,
  ProductionLineCompleted,
  ProductionLineShutdownError,

  ProductionLineNotInProcess,
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
