#ifndef LASTBOXSTICKERPRINTCOMMAND_H
#define LASTBOXSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class PrintLastBoxStickerCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_PRINTLASTBOXSTICKER_NAME;
  const size_t CommandSize = COMMAND_PRINTLASTBOXSTICKER_SIZE;
  ReturnStatus Status;

 public:
  explicit PrintLastBoxStickerCommand(const QString& name);
  ~PrintLastBoxStickerCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(PrintLastBoxStickerCommand)

 signals:
  void printLastBoxSticker_signal(ReturnStatus& status);
};

#endif  // LASTBOXSTICKERPRINTCOMMAND_H
