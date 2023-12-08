#ifndef TRANSPONDERRELEASECOMMAND_H
#define TRANSPONDERRELEASECOMMAND_H

#include "abstract_client_command.h"
#include "abstract_firmware_generation_system.h"

class TransponderReleaseCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "transponder_release";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;
  std::unique_ptr<QByteArray> Firmware;

  std::shared_ptr<AbstractFirmwareGenerationSystem> Generator;

 public:
  explicit TransponderReleaseCommand(
      const QString& name,
      const std::shared_ptr<AbstractFirmwareGenerationSystem> generator);
  ~TransponderReleaseCommand();

  // AbstractClientCommand interface
 public:
  virtual ReturnStatus process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(TransponderReleaseCommand)

 signals:
  void releaseTransponder_signal(const StringDictionary& param,
                                 const StringDictionary& result,
                                 ReturnStatus& status);
};

#endif  // TRANSPONDERRELEASECOMMAND_H
