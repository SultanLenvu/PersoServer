#ifndef TRANSPONDER_SEED_MODEL_H
#define TRANSPONDER_SEED_MODEL_H

#include <QAbstractTableModel>
#include <QMap>
#include <QString>

class TransponderSeedModel : public QAbstractTableModel {
  Q_OBJECT
 private:
  const QMap<QString, QString>* Attributes;
  const QMap<QString, QString>* MasterKeys;

 public:
  explicit TransponderSeedModel(QObject* parent = nullptr);
  ~TransponderSeedModel();

  void build(const QMap<QString, QString>* attributes,
             const QMap<QString, QString>* masterKeys);
  void clear(void);
  bool isEmpty(void);
  const QMap<QString, QString>* attributes(void) const;
  const QMap<QString, QString>* masterKeys(void) const;

  // Методы модели
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  void deleteAll(void);
};

#endif  // TRANSPONDER_SEED_MODEL_H
