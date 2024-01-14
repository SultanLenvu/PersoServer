#ifndef CONFIRMTRANSPONDERRERELEASECOMMAND_H_
#define CONFIRMTRANSPONDERRERELEASECOMMAND_H_

#include "abstract_client_command.h"
#include "definitions.h"

class ConfirmTransponderRereleaseCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_CONFIRMTRANSPONDERRERELEASE_NAME;
  const size_t CommandSize = COMMAND_CONFIRMTRANSPONDERRERELEASE_SIZE;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;

 public:
  explicit ConfirmTransponderRereleaseCommand(const QString& name);
  ~ConfirmTransponderRereleaseCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ConfirmTransponderRereleaseCommand)

 signals:
  void confirmTransponderRerelease_signal(const StringDictionary& param,
                                          ReturnStatus& status);
};

#endif  // CONFIRMTRANSPONDERRERELEASECOMMAND_H_
