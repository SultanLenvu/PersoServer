#ifndef TRANSPONDER_SEED_H
#define TRANSPONDER_SEED_H

#include <QAbstractTableModel>
#include <QHash>
#include <QString>

class TransponderSeed : public QObject {
  Q_OBJECT
 private:
  const QHash<QString, QString>* Attributes;
  const QHash<QString, QString>* MasterKeys;

 public:
  explicit TransponderSeed(QObject* parent = nullptr);
  ~TransponderSeed();

  void build(const QHash<QString, QString>* attributes,
             const QHash<QString, QString>* masterKeys);
  void clear(void);
  bool isEmpty(void);
  const QHash<QString, QString>* attributes(void) const;
  const QHash<QString, QString>* masterKeys(void) const;

 private:
  Q_DISABLE_COPY(TransponderSeed);
  void deleteAll(void);
};

#endif  // TRANSPONDER_SEED_H
