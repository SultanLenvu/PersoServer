#ifndef CONFIRMTRANSPONDERRELEASECOMMAND_H_
#define CONFIRMTRANSPONDERRELEASECOMMAND_H_

#include "abstract_client_command.h"
#include "definitions.h"

class ConfirmTransponderReleaseCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_CONFIRMTRANSPONDERRELEASE_NAME;
  const size_t CommandSize = COMMAND_CONFIRMTRANSPONDERRELEASE_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit ConfirmTransponderReleaseCommand(const QString& name);
  ~ConfirmTransponderReleaseCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ConfirmTransponderReleaseCommand)

 signals:
  void confirmTransponderRelease_signal(const StringDictionary& param,
                                        ReturnStatus& status);
};

#endif  // CONFIRMTRANSPONDERRELEASECOMMAND_H_
