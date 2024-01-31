#ifndef REFUNDCURRENTBOXCOMMAND_H
#define REFUNDCURRENTBOXCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class RefundCurrentBoxCommand : public AbstractClientCommand
{
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_REFUNDCURRENTBOX_NAME;
  const size_t CommandSize = COMMAND_REFUNDCURRENTBOX_SIZE;

  ReturnStatus Status;

 public:
  explicit RefundCurrentBoxCommand(const QString& name);
  ~RefundCurrentBoxCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RefundCurrentBoxCommand)

 signals:
  void refundCurrentBox_signal(ReturnStatus& status);
};

#endif // REFUNDCURRENTBOXCOMMAND_H
