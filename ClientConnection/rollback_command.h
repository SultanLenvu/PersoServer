#ifndef ROLLBACKCOMMAND_H
#define ROLLBACKCOMMAND_H

#include "abstract_client_command.h"

class RollbackCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "rollback";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit RollbackCommand(const QString& name);
  ~RollbackCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RollbackCommand)

 signals:
  void rollback_signal(const StringDictionary& data,
                       StringDictionary& result,
                       ReturnStatus& status);
};

#endif  // ROLLBACKCOMMAND_H
