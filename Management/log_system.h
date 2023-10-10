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
  bool ConsoleOutputEnable;
  QTextStream* ConsoleOutput;

  bool UdpOutputEnable;
  QHostAddress DestIp;
  uint32_t DestPort;
  QUdpSocket* LogSocket;

 public:
  LogSystem(QObject* parent);
  ~LogSystem();

 public slots:
  void generate(const QString& log) const;

 private:
  Q_DISABLE_COPY(LogSystem);
  void loadSettings(void);

  void writeToUdp(const QString& log) const;
  void writeToConsole(const QString& log) const;
};

//==================================================================================

#endif  // LOGSYSTEM_H
