#ifndef DATABASBUFFER_H
#define DATABASBUFFER_H

#include <QAbstractTableModel>

class DatabaseBuffer : public QAbstractTableModel {
  Q_OBJECT

 private:
  QVector<QString>* Headers;
  QVector<QVector<QString>*>* Data;

 public:
  explicit DatabaseBuffer(QObject* parent);
  ~DatabaseBuffer();

  void build(QVector<QString>* headers, QVector<QVector<QString>*>* data);
  void clear(void);
  void log(void);

  // Функционал модели
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

 signals:
  void logging(const QString& log);
};

#endif // DATABASBUFFER_H
