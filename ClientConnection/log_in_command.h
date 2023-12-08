#ifndef LogInCommand_H
#define LogInCommand_H

#include "abstract_client_command.h"

class LogInCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "log_in";
  const size_t CommandSize = 3;

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
  void logIn_signal(const StringDictionary& param,
                    const StringDictionary& result,
                    ReturnStatus& status);

  void authorized(const QString& login, const QString& password);
};

#endif  // LogInCommand_H
