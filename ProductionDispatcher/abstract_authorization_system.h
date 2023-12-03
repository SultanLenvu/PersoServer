#ifndef ABSTRACTAUTHORIZATIONSYSTEM_H
#define ABSTRACTAUTHORIZATIONSYSTEM_H

#include <QObject>

class AbstractAuthorizationSystem : public QObject {
  Q_OBJECT
 public:
  explicit AbstractAuthorizationSystem(QObject* parent = nullptr);

 signals:
  void logging(const QString&);
};

#endif  // ABSTRACTAUTHORIZATIONSYSTEM_H
