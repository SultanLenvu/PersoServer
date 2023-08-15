#include "gui_delegates.h"

/* Делегат для изменения цвета таблиц, отображающих результаты тестирования */
//==================================================================================

void TestTableView_ColorDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                        const QModelIndex& index) const
{
  // Получаем данные из модели
  QString text = index.data(Qt::DisplayRole).toString();

  // Определяем цвет ячейки в зависимости от содержимого
  if (text == "Пройден")
    painter->fillRect(option.rect, Qt::green);
  else if (text == "Провален")
    painter->fillRect(option.rect, Qt::red);
  else
    painter->fillRect(option.rect, Qt::white);

  // Рисуем текст ячейки
  QStyledItemDelegate::paint(painter, option, index);
}

QSize TestTableView_ColorDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  QString text = index.data(Qt::DisplayRole).toString();
  QFont font = option.font;
  QFontMetrics fm(font);
  QSize textSize = fm.size(Qt::TextSingleLine, text);
  size.setWidth(textSize.width() + 20);
  size.setHeight(textSize.height() + 10);
  return size;
}

//==================================================================================
