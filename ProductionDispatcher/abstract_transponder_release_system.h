#ifndef ABSTRACTTRANSPONDERRELEASESYSTEM_H
#define ABSTRACTTRANSPONDERRELEASESYSTEM_H

#include <QObject>

class AbstractTransponderReleaseSystem : public QObject {
  Q_OBJECT
 public:
  explicit AbstractTransponderReleaseSystem(QObject* parent = nullptr);

 signals:
  void logging(const QString&);
};

#endif  // ABSTRACTTRANSPONDERRELEASESYSTEM_H
