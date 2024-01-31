#ifndef ROLLBACKTRANSPONDERCOMMAND_H_
#define ROLLBACKTRANSPONDERCOMMAND_H_

#include "abstract_client_command.h"
#include "definitions.h"

class RollbackTransponderCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_ROLLBACKTRANSPONDER_NAME;
  const size_t CommandSize = COMMAND_ROLLBACKTRANSPONDER_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit RollbackTransponderCommand(const QString& name);
  ~RollbackTransponderCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RollbackTransponderCommand)

 signals:
  void rollbackTransponder_signal(ReturnStatus& status);
};

#endif  // ROLLBACKTRANSPONDERCOMMAND_H_
