#ifndef ABSTRACTFIRMWAREGENERATIONSYSTEM_H
#define ABSTRACTFIRMWAREGENERATIONSYSTEM_H

#include <QObject>

#include "General/types.h"

class AbstractFirmwareGenerationSystem : public QObject {
  Q_OBJECT
 public:
  explicit AbstractFirmwareGenerationSystem(const QString& name);
  virtual ~AbstractFirmwareGenerationSystem();

  virtual bool init(void) = 0;
  virtual ReturnStatus generate(const StringDictionary& seed,
                                QByteArray& assembledFirmware) = 0;

 private:
  AbstractFirmwareGenerationSystem();
  Q_DISABLE_COPY_MOVE(AbstractFirmwareGenerationSystem)

 signals:
  void logging(const QString& log);
};

#endif  // ABSTRACTFIRMWAREGENERATIONSYSTEM_H
