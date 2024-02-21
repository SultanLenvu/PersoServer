#ifndef UdpLogBackend_H
#define UdpLogBackend_H

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

#include "Log/log_backend.h"

class UdpLogBackend : public LogBackend {
  Q_OBJECT
 private:
  bool LogEnable;
  QHostAddress DestIp;
  uint32_t DestPort;
  QUdpSocket* LogSocket;

 public:
  explicit UdpLogBackend(const QString& name);

  virtual void writeLogMessage(const QString& str) override;

 private:
  Q_DISABLE_COPY_MOVE(UdpLogBackend);
  void loadSettings(void);
};

#endif /* UdpLogBackend_H */
