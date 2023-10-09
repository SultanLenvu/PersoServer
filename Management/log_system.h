#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#include <QHostAddress>
#include <QObject>
#include <QSettings>
#include <QTextStream>
#include <QTime>
#include <QUdpSocket>

#include "General/definitions.h"

/* Глобальная система логгирования */
//==================================================================================

class LogSystem : public QObject {
  Q_OBJECT

 private:
  bool Enable;

  QHostAddress SendIp;
  uint32_t SendPort;
  QHostAddress DestIp;
  uint32_t DestPort;
  QUdpSocket* LogSocket;

 public:
  LogSystem(QObject* parent);
  ~LogSystem();

 public slots:
  void generate(const QString& log);

 private:
  void loadSettings(void);
};

//==================================================================================

#endif  // LOGSYSTEM_H
