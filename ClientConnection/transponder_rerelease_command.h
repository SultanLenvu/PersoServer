#ifndef TRANSPONDERRERELEASECOMMAND_H
#define TRANSPONDERRERELEASECOMMAND_H

#include "abstract_client_command.h"
#include "abstract_firmware_generation_system.h"

class TransponderRereleaseCommand : public AbstractClientCommand {
 private:
  const QString CommandName = "transponder_release";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;
  std::unique_ptr<QByteArray> Firmware;

  std::shared_ptr<AbstractFirmwareGenerationSystem> Generator;

 public:
  explicit TransponderRereleaseCommand(
      const QString& name,
      const std::shared_ptr<AbstractFirmwareGenerationSystem> generator);
  ~TransponderRereleaseCommand();

  // AbstractClientCommand interface
 public:
  virtual ReturnStatus process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(TransponderRereleaseCommand)

 signals:
  void rereleaseTransponder_signal(const StringDictionary& param,
                                   const StringDictionary& result,
                                   ReturnStatus& status);
};

#endif  // TRANSPONDERRERELEASECOMMAND_H
