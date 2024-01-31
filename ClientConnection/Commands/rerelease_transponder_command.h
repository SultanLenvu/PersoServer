#ifndef RERELEASECOMMAND_H
#define RERELEASECOMMAND_H

#include "abstract_client_command.h"
#include "definitions.h"

class RereleaseTransponderCommand : public AbstractClientCommand {
  Q_OBJECT

 private:
  const QString CommandName = COMMAND_RERELEASETRANSPONDER_NAME;
  const size_t CommandSize = COMMAND_RERELEASETRANSPONDER_SIZE;

  StringDictionary Parameters;
  ReturnStatus Status;
  std::unique_ptr<QByteArray> Firmware;

 public:
  explicit RereleaseTransponderCommand(const QString& name);
  ~RereleaseTransponderCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RereleaseTransponderCommand)

 signals:
  void rereleaseTransponder_signal(const StringDictionary& param,
                                   QByteArray& firmware,
                                   ReturnStatus& status);
};

#endif  // RERELEASECOMMAND_H
