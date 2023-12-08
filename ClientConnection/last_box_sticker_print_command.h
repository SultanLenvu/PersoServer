#ifndef LASTBOXSTICKERPRINTCOMMAND_H
#define LASTBOXSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"

class LastBoxStickerPrintCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "last_box_sticker_print";
  const size_t CommandSize = 1;
  ReturnStatus Status;

 public:
  explicit LastBoxStickerPrintCommand(const QString& name);
  ~LastBoxStickerPrintCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(LastBoxStickerPrintCommand)

 signals:
  void printLastBoxSticker_signal(ReturnStatus& status);
};

#endif  // LASTBOXSTICKERPRINTCOMMAND_H
