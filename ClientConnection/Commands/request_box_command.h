#ifndef REQUESTBOXCOMMAND_H
#define REQUESTBOXCOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class RequestBoxCommand : public AbstractClientCommand
{
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_REQUESTBOX_NAME;
  const size_t CommandSize = COMMAND_REQUESTBOX_SIZE;

  ReturnStatus Status;

 public:
  explicit RequestBoxCommand(const QString& name);
  ~RequestBoxCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RequestBoxCommand)

 signals:
  void requestBox_signal(ReturnStatus& status);
};

#endif // REQUESTBOXCOMMAND_H
