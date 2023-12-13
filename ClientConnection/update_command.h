#ifndef UpdateCommand_H
#define UpdateCommand_H

#include "abstract_client_command.h"

class UpdateCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "update";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit UpdateCommand(const QString& name);
  ~UpdateCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(UpdateCommand)

 signals:
  void update_signal(const StringDictionary& param,
                     StringDictionary& context,
                     ReturnStatus& status);
};

#endif  // UpdateCommand_H
