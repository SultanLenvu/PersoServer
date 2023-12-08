#ifndef SHUTDOWNCOMMAND_H
#define SHUTDOWNCOMMAND_H

#include "abstract_client_command.h"

class ShutdownCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "shutdown";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit ShutdownCommand(const QString& name);
  ~ShutdownCommand();

  // AbstractClientCommand interface
 public:
  virtual ReturnStatus process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ShutdownCommand)

 signals:
  void shutdownProductionLine_signal(const StringDictionary& param,
                                     ReturnStatus& status);
};

#endif  // SHUTDOWNCOMMAND_H
