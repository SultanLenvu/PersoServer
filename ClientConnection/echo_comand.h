#ifndef ECHOCOMMAND_H
#define ECHOCOMMAND_H

#include "abstract_client_command.h"

class EchoCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "echo";
  const size_t CommandSize = 2;

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

#endif  // ECHOCOMMAND_H
