#ifndef LASTPALLETSTICKERPRINTCOMMAND_H
#define LASTPALLETSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"

class LastPalletStickerPrintCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "last_pallet_sticker_print";
  const size_t CommandSize = 1;
  ReturnStatus Status;

 public:
  explicit LastPalletStickerPrintCommand(const QString& name);
  ~LastPalletStickerPrintCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(LastPalletStickerPrintCommand)

 signals:
  void printLastPalletSticker_signal(ReturnStatus& status);
};

#endif  // LASTPALLETSTICKERPRINTCOMMAND_H
