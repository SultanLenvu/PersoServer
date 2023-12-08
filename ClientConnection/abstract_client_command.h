#ifndef ABSTRACTCLIENTCOMMAND_H
#define ABSTRACTCLIENTCOMMAND_H

#include <QJsonObject>
#include <QObject>

#include <General/types.h>

class AbstractClientCommand : public QObject {
  Q_OBJECT
 public:
  explicit AbstractClientCommand(const QString& name);
  virtual ~AbstractClientCommand();

  virtual void process(const QJsonObject& command) = 0;
  virtual void generateResponse(QJsonObject& response) = 0;
  virtual void reset(void) = 0;

 private:
  AbstractClientCommand();
  Q_DISABLE_COPY_MOVE(AbstractClientCommand)

 signals:
  
};

#endif  // ABSTRACTCLIENTCOMMAND_H
