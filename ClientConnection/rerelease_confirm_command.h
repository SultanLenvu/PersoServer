#ifndef RERELEASECONFIRMCOMMAND_H
#define RERELEASECONFIRMCOMMAND_H

#include "abstract_client_command.h"

class RereleaseConfirmCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "release_confirm";
  const size_t CommandSize = 5;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit RereleaseConfirmCommand(const QString& name);
  ~RereleaseConfirmCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RereleaseConfirmCommand)

 signals:
  void confirmRerelease_signal(const StringDictionary& param,
                               ReturnStatus& status);
};

#endif  // RERELEASECONFIRMCOMMAND_H
