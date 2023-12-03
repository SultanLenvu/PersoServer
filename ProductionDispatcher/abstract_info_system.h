#ifndef AbstractInfoSystem_H
#define AbstractInfoSystem_H

#include <QObject>

class AbstractInfoSystem : public QObject {
  Q_OBJECT
 public:
  explicit AbstractInfoSystem(QObject* parent = nullptr);

 signals:
  void logging(const QString&);
};

#endif  // AbstractInfoSystem_H
