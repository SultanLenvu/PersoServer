#include "file_log_backend.h"

FileLogBackend::FileLogBackend(QObject* parent) : LogBackend(parent) {
  setObjectName("FileLogBackend");
  loadSettings();
  initialize();
}

FileLogBackend::~FileLogBackend() {}

void FileLogBackend::writeLogLine(const QString& str) {
  if (LogEnable) {
    LogTextStream << str << "\n";
  }
}

void FileLogBackend::clear() { /* No-op */
}

void FileLogBackend::loadSettings() {
  QSettings settings;

  CurrentLogDir = QCoreApplication::applicationDirPath() + "/logs";
  LogEnable = settings.value("log_system/file_log_enable").toBool();
  LogFileMaxNumber = settings.value("log_system/log_file_max_number").toInt();
}

void FileLogBackend::initialize() {
  QDir logDir;
  if (!logDir.mkpath(QCoreApplication::applicationDirPath() + "/logs")) {
    LogEnable = false;
    QTextStream(stdout) << "Не удалось создать директорию для логгирования. ";
    return;
  }

  if (CurrentLogFile.isOpen()) {
    CurrentLogFile.close();
  }
  CurrentLogFile.setFileName(
      CurrentLogDir + "/log " +
      QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"));
  if (!CurrentLogFile.open(QIODevice::WriteOnly)) {
    LogEnable = false;
    QTextStream(stdout) << "Не удалось открыть файл для логгирования. ";
    return;
  }

  removeOldestLogFiles();

  LogEnable = true;
  LogTextStream.setDevice(&CurrentLogFile);
}

void FileLogBackend::removeOldestLogFiles() {
  QDir directory(CurrentLogDir);

  // Получаем список файлов в директории
  QFileInfoList fileList =
      directory.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time);

  if (fileList.size() > LogFileMaxNumber) {
    int filesToDeleteCount = fileList.size() - LogFileMaxNumber;

    // Удаляем самые старые файлы
    for (int i = 0; i < filesToDeleteCount; ++i) {
      const QFileInfo& fileInfo = fileList.at(i);
      QString filePath = fileInfo.absoluteFilePath();

      QFile::remove(filePath);
    }
  }
}
