#ifndef ABSTRACTPRODUCTIONDISPATCHER_H
#define ABSTRACTPRODUCTIONDISPATCHER_H

#include <QObject>

class AbstractProductionDispatcher : public QObject {
  Q_OBJECT
 public:
  explicit AbstractProductionDispatcher(QObject* parent = nullptr);

 signals:
  void logging(const QString& log);
};

#endif  // ABSTRACTPRODUCTIONDISPATCHER_H
