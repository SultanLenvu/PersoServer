#ifndef LAUNCHCOMMAND_H
#define LAUNCHCOMMAND_H

#include "abstract_client_command.h"

class LaunchCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "launch";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit LaunchCommand(const QString& name);
  ~LaunchCommand();

  // AbstractClientCommand interface
 public:
  virtual ReturnStatus process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(LaunchCommand)

 signals:
  void launchProductionLine_signal(const StringDictionary& param,
                                   const StringDictionary& result,
                                   ReturnStatus& status);
};

#endif  // LAUNCHCOMMAND_H
