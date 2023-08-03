#ifndef DTR_LOGSYSTEM_H
#define DTR_LOGSYSTEM_H

#include <QObject>
#include <QTime>

/* Глобальная система логгирования */
//==================================================================================

class GlobalLogSystem : public QObject {
  Q_OBJECT

 private:
  bool EnableIndicator;

 public:
  GlobalLogSystem(QObject* parent);
  ~GlobalLogSystem();

 public:
  void clear(void);
  void setEnable(bool option);

 public slots:
  void dtrLogging(const QString& log);
  void managerLogging(const QString& log);

 private:
  void generalLogging(QString sender, QString log);

 signals:
  void displayLogRequest(const QString& logData);
  void clearLogDisplayRequest(void);
};

//==================================================================================

#endif  // DTR_LOGSYSTEM_H
