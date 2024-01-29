#ifndef GETPRODUCTIONLINEDATACOMMAND_H
#define GETPRODUCTIONLINEDATACOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class GetProductionLineDataCommand : public AbstractClientCommand
{
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_GETPRODUCTIONLINEDATA_NAME;
  const size_t CommandSize = COMMAND_GETNPRODUCTIONLINEDATA_SIZE;

  StringDictionary Parameters;
  StringDictionary ProductionLineData;
  ReturnStatus Status;

 public:
  explicit GetProductionLineDataCommand(const QString& name);
  ~GetProductionLineDataCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(GetProductionLineDataCommand)

 signals:
  void getProductionLineData_signal(StringDictionary& context,
                                    ReturnStatus& status);
};

#endif // GETPRODUCTIONLINEDATACOMMAND_H
