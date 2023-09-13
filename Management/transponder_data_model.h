#ifndef TRANSPONDER_DATA_MODEL_H
#define TRANSPONDER_DATA_MODEL_H

#include <QAbstractTableModel>
#include <QMap>
#include <QMutex>
#include <QString>

class TransponderDataModel : public QAbstractTableModel {
  Q_OBJECT
 private:
  QMap<QString, QString>* Data;

  QMutex Mutex;

 public:
  explicit TransponderDataModel(QObject* parent = nullptr);

  void build(QMap<QString, QString>* data);
  void clear(void);
  bool isEmpty(void);
  const QMap<QString, QString>* getMap(void);

  // Функционал модели
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  void deleteData(void);
};

#endif  // TRANSPONDER_DATA_MODEL_H
