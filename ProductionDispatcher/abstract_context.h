#ifndef ABSTRACTCONTEXT_H
#define ABSTRACTCONTEXT_H

#include <QObject>

class AbstractContext
{
 public:
  AbstractContext();
  virtual ~AbstractContext();

  virtual void clear(void) = 0;
  virtual void stash(void) = 0;
  virtual void applyStash(void) = 0;

 private:
  Q_DISABLE_COPY_MOVE(AbstractContext)
};

#endif // ABSTRACTCONTEXT_H
