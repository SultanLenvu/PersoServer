#ifndef LogOutCommand_H
#define LogOutCommand_H

#include "abstract_client_command.h"

class LogOutCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "log_out";
  const size_t CommandSize = 3;

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
  void logOut_signal(const StringDictionary& param, ReturnStatus& status);
  void deauthorized(void);
};

#endif  // LogOutCommand_H
