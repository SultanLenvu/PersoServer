#ifndef DATABASBUFFER_H
#define DATABASBUFFER_H

#include <QAbstractTableModel>
#include <QMutex>
#include <QMutexLocker>

class DatabaseTableModel : public QAbstractTableModel {
  Q_OBJECT

 private:
  QVector<QString>* Headers;
  QVector<QVector<QString>*>* Data;

  QMutex Mutex;

 public:
  explicit DatabaseTableModel(QObject* parent);
  ~DatabaseTableModel();

  void build(QVector<QString>* headers, QVector<QVector<QString>*>* data);
  void clear(void);
  bool isEmpty(void);

  // Функционал модели
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 private:
  void deleteAll(void);
};

#endif // DATABASBUFFER_H
