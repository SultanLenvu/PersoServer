#ifndef LASTPALLETSTICKERPRINTCOMMAND_H
#define LASTPALLETSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class PrintLastPalletStickerCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_PRINTLASTPALLETSTICKER_NAME;
  const size_t CommandSize = COMMAND_PRINTLASTPALLETSTICKER_SIZE;
  ReturnStatus Status;

 public:
  explicit PrintLastPalletStickerCommand(const QString& name);
  ~PrintLastPalletStickerCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(PrintLastPalletStickerCommand)

 signals:
  void printLastPalletSticker_signal(ReturnStatus& status);
};

#endif  // LASTPALLETSTICKERPRINTCOMMAND_H
