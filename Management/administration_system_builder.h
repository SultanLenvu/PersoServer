#ifndef ADMINISTRATION_SYSTEM_BUILDER_H
#define ADMINISTRATION_SYSTEM_BUILDER_H

#include <QObject>
#include <QThread>

#include "administration_system.h"

class AdministrationSystemBuilder : public QObject {
  Q_OBJECT
 private:
  AdministrationSystem* Administrator;

 public:
  explicit AdministrationSystemBuilder(void);
  AdministrationSystem* buildedObject() const;

 public slots:
  void build();

 signals:
  void completed(void);
};

#endif // ADMINISTRATION_SYSTEM_BUILDER_H
