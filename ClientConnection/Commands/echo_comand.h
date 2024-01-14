#ifndef ECHOCOMMAND_H_
#define ECHOCOMMAND_H_

#include "abstract_client_command.h"
#include "definitions.h"

class EchoCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_ECHO_NAME;
  const size_t CommandSize = COMMAND_ECHO_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit EchoCommand(const QString& name);
  ~EchoCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(EchoCommand)
};

#endif  // ECHOCOMMAND_H_
