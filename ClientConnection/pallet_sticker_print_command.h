#ifndef PALLETSTICKERPRINTCOMMAND_H
#define PALLETSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"

class PalletStickerPrintCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "pallet_sticker_print";
  const size_t CommandSize = 2;
  StringDictionary Parameters;
  ReturnStatus Status;

 public:
  explicit PalletStickerPrintCommand(const QString& name);
  ~PalletStickerPrintCommand();

  // AbstractClientCommand interface
 public:
  virtual ReturnStatus process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(PalletStickerPrintCommand)

 signals:
  void printPalletSticker_signal(const StringDictionary& data,
                                 ReturnStatus& status);
};

#endif  // PALLETSTICKERPRINTCOMMAND_H
