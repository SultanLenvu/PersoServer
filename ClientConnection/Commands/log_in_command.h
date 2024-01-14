#ifndef LogInCommand_H
#define LogInCommand_H

#include "abstract_client_command.h"
#include "definitions.h"

class LogInCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_LOGIN_NAME;
  const size_t CommandSize = COMMAND_LOGIN_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit LogInCommand(const QString& name);
  ~LogInCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(LogInCommand)

 signals:
  void launchProductionLine_signal(ReturnStatus& status);
  void authorized();
};

#endif  // LogInCommand_H
