#ifndef GLOBALCONTEXT_H
#define GLOBALCONTEXT_H

#include <QHash>
#include <QObject>

#include "ProductionDispatcher/abstract_authorization_system.h"
#include "ProductionDispatcher/abstract_info_system.h"
#include "ProductionDispatcher/abstract_production_dispatcher.h"
#include "ProductionDispatcher/abstract_transponder_release_system.h"

class GlobalContext : public QObject {
  Q_OBJECT
 private:
  QHash<QString, std::unique_ptr<QObject>> GlobalObjects;

 public:
  ~GlobalContext();
  static GlobalContext* instance(void);

  void registerObject(std::unique_ptr<QObject> obj);
  const QObject* getObject(const QString& name);

  // Фабричные методы
  AbstractProductionDispatcher* createProductionDispatcher(QObject* parent);
  AbstractAuthorizationSystem* createAuthorizationSystem(QObject* parent);
  AbstractInfoSystem* createInfoSystem(QObject* parent);
  AbstractTransponderReleaseSystem* createTransponderReleaseSystem(
      QObject* parent);

 private:
  explicit GlobalContext();
  explicit GlobalContext(QObject* parent);
  Q_DISABLE_COPY_MOVE(GlobalContext);

 signals:
};

#endif  // GLOBALCONTEXT_H
