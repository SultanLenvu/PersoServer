#ifndef RELEASECONFIRMCOMMAND_H
#define RELEASECONFIRMCOMMAND_H

#include "abstract_client_command.h"

class ReleaseConfirmCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "release_confirm";
  const size_t CommandSize = 4;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit ReleaseConfirmCommand(const QString& name);
  ~ReleaseConfirmCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ReleaseConfirmCommand)

 signals:
  void confirmRelease_signal(const StringDictionary& param,
                             StringDictionary& result,
                             ReturnStatus& status);
};

#endif  // RELEASECONFIRMCOMMAND_H
