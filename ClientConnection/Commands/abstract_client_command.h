#ifndef ABSTRACTCLIENTCOMMAND_H
#define ABSTRACTCLIENTCOMMAND_H

#include <QJsonObject>
#include <QObject>

#include <types.h>
#include "production_context.h"

class AbstractClientCommand : public QObject {
  Q_OBJECT

 protected:
  std::shared_ptr<ProductionContext> Context;

 public:
  explicit AbstractClientCommand(const QString& name);
  virtual ~AbstractClientCommand();

  std::shared_ptr<ProductionContext> context();
  void setContext(std::shared_ptr<ProductionContext> context);

  virtual void process(const QJsonObject& command) = 0;
  virtual void generateResponse(QJsonObject& response) = 0;
  virtual void reset(void) = 0;

 protected:
  void sendLog(const QString& log);

 private:
  AbstractClientCommand();
  Q_DISABLE_COPY_MOVE(AbstractClientCommand)

 signals:
  void logging(const QString& log);
};

#endif  // ABSTRACTCLIENTCOMMAND_H
