#include "CheckBoxDelegate.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

CheckBoxDelegate::CheckBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {}

void CheckBoxDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  // 获取数据
  Qt::CheckState checkState =
      static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
  QString text = index.data(Qt::DisplayRole).toString();

  // 绘制复选框
  int boxSize = 20;
  int x = option.rect.x() + 12;
  int y = option.rect.y() + (option.rect.height() - boxSize) / 2;
  QRect boxRect(x, y, boxSize, boxSize);
  drawCustomCheckBox(painter, boxRect, checkState,
                     option.state & QStyle::State_MouseOver);

  // 绘制文字
  QRect textRect = option.rect.adjusted(x + boxSize + 10, 0, -5, 0);
  painter->setPen(QColor(40, 40, 45));
  painter->setFont(option.font);
  painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);

  painter->restore();
}

void CheckBoxDelegate::drawCustomCheckBox(QPainter *painter, const QRect &rect,
                                          Qt::CheckState state,
                                          bool hover) const {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);

  const qreal borderRadius = 5.0;
  const QColor accentColor(0, 122, 255);
  const QColor borderColor(200, 200, 205);
  const QColor hoverColor(245, 245, 247);

  QRectF boxRect = QRectF(rect).adjusted(0.5, 0.5, -0.5, -0.5);

  if (state == Qt::Checked) {
    painter->setBrush(accentColor);
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(boxRect, borderRadius, borderRadius);

    QPen checkPen(Qt::white, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(checkPen);
    QPainterPath path;
    path.moveTo(boxRect.left() + boxRect.width() * 0.28,
                boxRect.top() + boxRect.height() * 0.5);
    path.lineTo(boxRect.left() + boxRect.width() * 0.44,
                boxRect.top() + boxRect.height() * 0.72);
    path.lineTo(boxRect.left() + boxRect.width() * 0.72,
                boxRect.top() + boxRect.height() * 0.32);
    painter->drawPath(path);
  } else {
    painter->setBrush(hover ? hoverColor : Qt::white);
    painter->setPen(QPen(hover ? accentColor : borderColor, 1.5));
    painter->drawRoundedRect(boxRect, borderRadius, borderRadius);
  }
  painter->restore();
}

QSize CheckBoxDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const {
  Q_UNUSED(index)
  return QSize(option.rect.width(), 34);
}
