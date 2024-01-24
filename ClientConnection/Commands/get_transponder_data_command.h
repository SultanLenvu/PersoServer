#ifndef GETTRANSPONDERDATACOMMAND_H
#define GETTRANSPONDERDATACOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class GetTransponderDataCommand : public AbstractClientCommand
{
  Q_OBJECT

 private:
  const QString CommandName = COMMAND_GETTRANSPONDERDATA_NAME;
  const size_t CommandSize = COMMAND_GETTRANSPONDERDATA_SIZE;

  StringDictionary Parameters;
  ReturnStatus Status;

 public:
  explicit GetTransponderDataCommand(const QString& name);
  ~GetTransponderDataCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(GetTransponderDataCommand)

 signals:
  void getTransponderData_signal(const StringDictionary& param,
                                 StringDictionary& data,
                                 ReturnStatus& status);
};

#endif // GETTRANSPONDERDATACOMMAND_H
