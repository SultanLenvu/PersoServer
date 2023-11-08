#ifndef SQLTABLERELATION_H
#define SQLTABLERELATION_H

#include <QObject>

class SqlTableRelation : public QObject {
  Q_OBJECT
 public:
  explicit SqlTableRelation(QObject* parent);
  ~SqlTableRelation();
};

#endif  // SQLTABLERELATION_H
