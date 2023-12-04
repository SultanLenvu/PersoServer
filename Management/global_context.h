#ifndef GLOBALCONTEXT_H
#define GLOBALCONTEXT_H

#include <QHash>
#include <QObject>

class GlobalContext : public QObject {
  Q_OBJECT
 private:
  QHash<QString, std::shared_ptr<QObject>> GlobalObjects;

 public:
  ~GlobalContext();
  static GlobalContext* instance(void);

  void registerObject(std::shared_ptr<QObject> obj);
  const std::shared_ptr<QObject> getObject(const QString& name);

 private:
  explicit GlobalContext();
  Q_DISABLE_COPY_MOVE(GlobalContext);

 signals:
};

#endif  // GLOBALCONTEXT_H
