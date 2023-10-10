#ifndef TRANSPONDER_SEED_H
#define TRANSPONDER_SEED_H

#include <QAbstractTableModel>
#include <QMap>
#include <QString>

class TransponderSeed : public QObject {
  Q_OBJECT
 private:
  const QMap<QString, QString>* Attributes;
  const QMap<QString, QString>* MasterKeys;

 public:
  explicit TransponderSeed(QObject* parent = nullptr);
  ~TransponderSeed();

  void build(const QMap<QString, QString>* attributes,
             const QMap<QString, QString>* masterKeys);
  void clear(void);
  bool isEmpty(void);
  const QMap<QString, QString>* attributes(void) const;
  const QMap<QString, QString>* masterKeys(void) const;

 private:
  Q_DISABLE_COPY(TransponderSeed);
  void deleteAll(void);
};

#endif  // TRANSPONDER_SEED_H
