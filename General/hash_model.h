#ifndef HASH_MODEL_H
#define HASH_MODEL_H

#include <QAbstractTableModel>
#include <QHash>
#include <QString>

class HashModel : public QAbstractTableModel
{
private:
  QHash<QString, QString> TransponderDataMatchTable;
  QHash<QString, QVariant> HashTable;

  QVector<QVariant> Values;
  QVector<QVariant> Headers;

public:
  explicit HashModel(QObject* parent);
  ~HashModel();

  void buildTransponderData(const QHash<QString, QString>* map);
  void clear(void);
  bool isEmpty(void) const;
  const QHash<QString, QVariant>* hash(void) const;

  // Методы модели
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section,
		      Qt::Orientation orientation,
		      int role = Qt::DisplayRole) const override;

private:
  Q_DISABLE_COPY_MOVE(HashModel)
  void createMatchTables(void);
};

#endif // HASH_MODEL_H
