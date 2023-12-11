#ifndef RERELEASECOMMAND_H
#define RERELEASECOMMAND_H

#include "abstract_client_command.h"

class RereleaseCommand : public AbstractClientCommand {
  Q_OBJECT

 private:
  const QString CommandName = "transponder_release";
  const size_t CommandSize = 4;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;
  std::unique_ptr<QByteArray> Firmware;

 public:
  explicit RereleaseCommand(const QString& name);
  ~RereleaseCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(RereleaseCommand)

 signals:
  void rerelease_signal(const StringDictionary& param,
                        StringDictionary& result,
                        ReturnStatus& status);
};

#endif  // RERELEASECOMMAND_H
