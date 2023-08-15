#ifndef OBUINITIALIZATOR_H
#define OBUINITIALIZATOR_H

#include <QObject>

#include "Database/database_controller_interface.h"

class ObuInitializator : public QObject {
  Q_OBJECT
 private:
  uint32_t CurrentSerialNumber;

 public:
  explicit ObuInitializator(QObject* parent);

  void getCurrentSerialNumber(void);
  void loadObuByPanList(const QString& path);
  void loadObuBySnPanList(const QString& path);

 signals:
};

#endif  // OBUINITIALIZATOR_H
