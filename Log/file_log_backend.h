#ifndef FILELOGBACKEND_H
#define FILELOGBACKEND_H

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QTextStream>

#include "log_backend.h"

class FileLogBackend : public LogBackend {
  Q_OBJECT
 private:
  bool LogEnable;
  int32_t LogFileMaxNumber;
  QString CurrentLogDir;
  QFile CurrentLogFile;
  QTextStream LogTextStream;

 public:
  explicit FileLogBackend(QObject* parent);
  ~FileLogBackend();

  virtual void writeLogLine(const QString& str) override;
  virtual void clear() override;

 private:
  Q_DISABLE_COPY(FileLogBackend);
  void loadSettings(void);
  void initialize(void);
  void removeOldestLogFiles(void);

 signals:
  void notifyAboutError(const QString& log);
};

#endif /* FILELOGBACKEND_H */
