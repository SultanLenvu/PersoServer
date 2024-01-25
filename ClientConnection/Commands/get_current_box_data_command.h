#ifndef GETCURRENTBOXDATACOMMAND_H
#define GETCURRENTBOXDATACOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class GetCurrentBoxDataCommand : public AbstractClientCommand
{
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_GETCURRENTBOXDATA_NAME;
  const size_t CommandSize = COMMAND_GETCURRENTBOXDATA_SIZE;

  StringDictionary BoxData;
  ReturnStatus Status;

 public:
  explicit GetCurrentBoxDataCommand(const QString& name);
  ~GetCurrentBoxDataCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(GetCurrentBoxDataCommand)

 signals:
  void getCurrentBoxData_signal(StringDictionary& data, ReturnStatus& status);
};

#endif // GETCURRENTBOXDATACOMMAND_H
