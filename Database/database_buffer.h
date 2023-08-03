#ifndef DATABASBUFFER_H
#define DATABASBUFFER_H

#include <QAbstractTableModel>

class DatabaseBuffer : public QAbstractTableModel {
  Q_OBJECT

 private:
  QVector<QVector<QString>> Buffer;

 public:
  explicit DatabaseBuffer(QObject* parent = nullptr);

  // Функционал модели
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
};

#endif // DATABASBUFFER_H
