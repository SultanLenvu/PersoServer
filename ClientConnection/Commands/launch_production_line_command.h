#ifndef LAUNCHPRODUCTIONLINE_H
#define LAUNCHPRODUCTIONLINE_H

#include "abstract_client_command.h"
#include "definitions.h"

class LaunchProductionLineCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_LAUNCHPRODUCTIONLINE_NAME;
  const size_t CommandSize = COMMAND_LAUNCHPRODUCTIONLINE_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit LaunchProductionLineCommand(const QString& name);
  ~LaunchProductionLineCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(LaunchProductionLineCommand)

 signals:
  void launchProductionLine_signal(ReturnStatus& status);
  void authorized();
};

#endif  // LAUNCHPRODUCTIONLINE_H
