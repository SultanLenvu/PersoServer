#ifndef LogOutCommand_H
#define LogOutCommand_H

#include "abstract_client_command.h"
#include "definitions.h"

class LogOutCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_LOGOUT_NAME;
  const size_t CommandSize = COMMAND_LOGOUT_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit LogOutCommand(const QString& name);
  ~LogOutCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(LogOutCommand)

 signals:
  void shutdownProductionLine_signal(ReturnStatus& status);
  void deauthorized(void);
};

#endif  // LogOutCommand_H
