#ifndef PSOBJECT_H
#define PSOBJECT_H

#include <QObject>

class PSObject : public QObject
{
  Q_OBJECT
 public:
  explicit PSObject(const QString& name);
  virtual ~PSObject();

 public slots:
  void applySettings(void);

 protected:
  void sendLog(const QString& log);

 private:
  PSObject();
  Q_DISABLE_COPY_MOVE(PSObject)
  void connectDependencies(void);

  virtual void loadSettings();

 signals:
  void logging(const QString& log);
};

#endif // PSOBJECT_H
