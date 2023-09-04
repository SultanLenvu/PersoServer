#ifndef TRANSPONDERRELEASESYSTEM_H
#define TRANSPONDERRELEASESYSTEM_H

#include <Database/database_controller.h>
#include <QWidget>

class TransponderReleaseSystem : public QWidget
{
  Q_OBJECT

 private:
  IDatabaseController* Database;

 public:
  explicit TransponderReleaseSystem(QWidget* parent,
                                    IDatabaseController* database);

  void confirmTransponderRelease(void);

 private:
  void loadSettings(void);

 signals:
  void logging(const QString& log) const;
};

#endif // TRANSPONDERRELEASESYSTEM_H
