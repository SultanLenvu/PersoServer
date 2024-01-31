#ifndef BOXSTICKERPRINTCOMMAND_H
#define BOXSTICKERPRINTCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class BoxStickerPrintCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_PRINTBOXSTICKER_NAME;
  const size_t CommandSize = COMMAND_PRINTBOXSTICKER_SIZE;

  StringDictionary Parameters;
  ReturnStatus Status;

 public:
  explicit BoxStickerPrintCommand(const QString& name);
  ~BoxStickerPrintCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(BoxStickerPrintCommand)

 signals:
  void printBoxSticker_signal(const StringDictionary& data,
                              ReturnStatus& status);
};

#endif  // BOXSTICKERPRINTCOMMAND_H
