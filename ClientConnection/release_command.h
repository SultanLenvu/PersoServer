#ifndef RELEASECOMMAND_H
#define RELEASECOMMAND_H

#include "abstract_client_command.h"

class ReleaseCommand : public AbstractClientCommand {
  Q_OBJECT
 private:
  const QString CommandName = "release";
  const size_t CommandSize = 3;

  StringDictionary Parameters;
  StringDictionary Result;
  ReturnStatus Status;
  std::unique_ptr<QByteArray> Firmware;

 public:
  explicit ReleaseCommand(const QString& name);
  ~ReleaseCommand();

  // AbstractClientCommand interface
 public:
  virtual void process(const QJsonObject& command) override;
  virtual void generateResponse(QJsonObject& response) override;
  virtual void reset(void) override;

 private:
  Q_DISABLE_COPY_MOVE(ReleaseCommand)

 signals:
  void release_signal(const StringDictionary& param,
                      StringDictionary& result,
                      ReturnStatus& status);
};

#endif  // RELEASECOMMAND_H
