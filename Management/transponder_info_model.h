#ifndef TRANSPONDERINFOMODEL_H
#define TRANSPONDERINFOMODEL_H

#include <QAbstractListModel>
#include <QMap>
#include <QMutex>
#include <QString>

class TransponderInfoModel : public QAbstractListModel {
  Q_OBJECT
 private:
  QMap<QString, QString>* Data;

  QMutex Mutex;

 public:
  explicit TransponderInfoModel(QObject* parent = nullptr);

  void build(QMap<QString, QString>* data);
  void clear(void);
  bool isEmpty(void);

  // Функционал модели
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  void deleteData(void);
};

#endif // TRANSPONDERINFOMODEL_H
