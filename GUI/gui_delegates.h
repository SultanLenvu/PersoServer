#ifndef GUI_DELEGATES_H
#define GUI_DELEGATES_H

#include <QStyledItemDelegate>
#include "qpainter.h"

/* Делегат для изменения цвета таблиц, отображающих результаты тестирования */
//==================================================================================

class TestTableView_ColorDelegate : public QStyledItemDelegate
{
public:
  explicit TestTableView_ColorDelegate(QObject* parent) : QStyledItemDelegate(parent)
  {
  }

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

//==================================================================================

#endif  // GUI_DELEGATES_H
