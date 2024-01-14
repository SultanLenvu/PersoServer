#ifndef RELEASETRANSPONDERCOMMAND_H_
#define RELEASETRANSPONDERCOMMAND_H_

#include "abstract_client_command.h"
#include "definitions.h"

class ReleaseTransponderCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = COMMAND_RELEASETRANSPONDER_NAME;
  const size_t CommandSize = COMMAND_RELEASETRANSPONDER_SIZE;

  StringDictionary Parameters;
  ReturnStatus Status;
  std::unique_ptr<QByteArray> Firmware;

 public:
  explicit ReleaseTransponderCommand(const QString& name);
  ~ReleaseTransponderCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ReleaseTransponderCommand)

 signals:
  void releaseTransponder_signal(QByteArray& firmware, ReturnStatus& status);
};

#endif  // RELEASETRANSPONDERCOMMAND_H_
