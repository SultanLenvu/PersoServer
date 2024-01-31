#ifndef COMPLETECURRENTBOXCOMMAND_H
#define COMPLETECURRENTBOXCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class CompleteCurrentBoxCommand : public AbstractClientCommand
{
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_COMPLETECURRENTBOX_NAME;
  const size_t CommandSize = COMMAND_COMPLETECURRENTBOX_SIZE;

  ReturnStatus Status;

 public:
  explicit CompleteCurrentBoxCommand(const QString& name);
  ~CompleteCurrentBoxCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(CompleteCurrentBoxCommand)

 signals:
  void completeCurrentBox_signal(ReturnStatus& status);
};

#endif // COMPLETECURRENTBOXCOMMAND_H
