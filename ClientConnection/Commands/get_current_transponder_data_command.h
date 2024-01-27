#ifndef GETCURRENTTRANSPONDERDATACOMMAND_H_
#define GETCURRENTTRANSPONDERDATACOMMAND_H_

#include "abstract_client_command.h"
#include "definitions.h"

class GetCurrentTransponderDataCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_GETCURRENTTRANSPONDERDATA_NAME;
  const size_t CommandSize = COMMAND_GETCURRENTTRANSPONDERDATA_SIZE;

  StringDictionary Parameters;
  StringDictionary TransponderData;
  ReturnStatus Status;

 public:
  explicit GetCurrentTransponderDataCommand(const QString& name);
  ~GetCurrentTransponderDataCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(GetCurrentTransponderDataCommand)

 signals:
  void getCurrentTransponderData_signal(StringDictionary& context,
                                        ReturnStatus& status);
};

#endif  // GETCURRENTTRANSPONDERDATACOMMAND_H_
