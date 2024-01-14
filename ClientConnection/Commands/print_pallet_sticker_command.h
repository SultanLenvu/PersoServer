#ifndef PALLETSTICKERPRINTCOMMAND_H
#define PALLETSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class PrintPalletStickerCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_PRINTPALLETSTICKER_NAME;
  const size_t CommandSize = COMMAND_PRINTPALLETSTICKER_SIZE;
  StringDictionary Parameters;
  ReturnStatus Status;

 public:
  explicit PrintPalletStickerCommand(const QString& name);
  ~PrintPalletStickerCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(PrintPalletStickerCommand)

 signals:
  void printPalletSticker_signal(const StringDictionary& data,
                                 ReturnStatus& status);
};

#endif  // PALLETSTICKERPRINTCOMMAND_H
