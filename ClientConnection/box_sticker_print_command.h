#ifndef BOXSTICKERPRINTCOMMAND_H
#define BOXSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"

class BoxStickerPrintCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "box_sticker_print";
  const size_t CommandSize = 2;

  StringDictionary Parameters;
  ReturnStatus Status;

 public:
  explicit BoxStickerPrintCommand(const QString& name);
  ~BoxStickerPrintCommand();

  // AbstractClientCommand interface
 public:
  virtual ReturnStatus process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(BoxStickerPrintCommand)

 signals:
  void printBoxSticker_signal(const StringDictionary& data,
                              ReturnStatus& status);
};

#endif  // BOXSTICKERPRINTCOMMAND_H
