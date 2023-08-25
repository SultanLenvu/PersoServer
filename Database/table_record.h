#ifndef DATABASETABLERECORD_H
#define DATABASETABLERECORD_H

#include <QDate>
#include <QObject>
#include <QPair>
#include <QString>
#include <QVector>

class UniversalTableRecord : public QObject {
  Q_OBJECT
 private:
  QVector<QPair<QString, QString>*> Attributes;

 public:
  explicit UniversalTableRecord(QObject* parent);
  ~UniversalTableRecord();

  void addAttribute(const QString& name, const QString& value);
  const QString& attributeValue(const uint32_t i) const;
  const QString& attributeName(const uint32_t i) const;
  void clear();

 signals:
};

#endif  // DATABASETABLERECORD_H
