#ifndef TRANSPONDER_DATA_MODEL_H
#define TRANSPONDER_DATA_MODEL_H

#include <QAbstractTableModel>
#include <QMap>
#include <QMutex>
#include <QString>

class TransponderSeedModel : public QAbstractTableModel {
  Q_OBJECT
 private:
  const QMap<QString, QString>* Data;

  QMutex Mutex;

 public:
  explicit TransponderSeedModel(QObject* parent = nullptr);
  ~TransponderSeedModel();

  void build(const QMap<QString, QString>* data);
  void clear(void);
  bool isEmpty(void);
  const QMap<QString, QString>* data(void);

  // Функционал модели
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  void deleteAll(void);
};

#endif  // TRANSPONDER_DATA_MODEL_H
