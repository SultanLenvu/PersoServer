#ifndef SHUTDOWNPRODUCTIONLINE_H
#define SHUTDOWNPRODUCTIONLINE_H

#include "abstract_client_command.h"
#include "definitions.h"

class ShutdownProductionLineCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_SHUTDOWNPRODUCTIONLINE_NAME;
  const size_t CommandSize = COMMAND_SHUTDOWNPRODUCTIONLINE_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit ShutdownProductionLineCommand(const QString& name);
  ~ShutdownProductionLineCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ShutdownProductionLineCommand)

 signals:
  void shutdownProductionLine_signal(ReturnStatus& status);
  void deauthorized(void);
};

#endif  // SHUTDOWNPRODUCTIONLINE_H
