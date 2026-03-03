#include "CheckBoxDelegate.h"
#include <QComboBox>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainterPath>
#include <QEvent>
#include <QDebug>
CheckBoxDelegate::CheckBoxDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void CheckBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //1.保存画笔状态
    painter->save();
    //2.获取复选框状态
    Qt::CheckState checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
    if (checkState != Qt::Checked && checkState != Qt::Unchecked)
    {
        checkState = Qt::Unchecked;
    }
    //3.绘制复选框
    int boxSize = 18;
    int x = option.rect.x() + 5;
    int y = option.rect.y() + (option.rect.height() - boxSize) / 2 ;
    QRect boxRect(x, y, boxSize, boxSize);
    drawCustomCheckBox(painter, boxRect, checkState,
                       option.state & QStyle::State_MouseOver);

    QRect textRect = option.rect.adjusted(boxSize + 20, 0, 0, 0);
    QString text = index.data(Qt::DisplayRole).toString();

    painter->setPen(Qt::black);
    painter->setFont(option.font);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text);
    painter->restore();
}

void CheckBoxDelegate::drawCustomCheckBox(QPainter *painter, const QRect &rect, Qt::CheckState state, bool hover) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    int borderRadius = 6;
    //创建裁剪路径（确保所有绘制都在圆角内）
    qDebug() << "ddadaddadadadd";
    QPainterPath clipPath;
    clipPath.addRoundedRect(rect, borderRadius, borderRadius);
    painter->setClipPath(clipPath);

    if (state == Qt::Checked) {
        QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
        gradient.setColorAt(0, QColor(QColor(52, 199, 89)));
        gradient.setColorAt(1, QColor(45, 180, 75));
        painter->fillRect(rect, gradient);  // 填充背景
        painter->setPen(QPen(QColor(255, 255, 255, 50), 1));
        painter->drawLine(rect.topLeft() + QPoint(1,1),
                         rect.topRight() + QPoint(-1,1));
    } else {
        if (hover) {
            painter->fillRect(rect, QColor(245, 245, 245));
        } else {
            painter->fillRect(rect, Qt::white);
        }

        painter->setPen(QPen(hover ? QColor(100,100,100) : QColor(200,200,200), 1.5));
        painter->drawRect(rect);  // 这个矩形会被裁剪成圆角
    }

    if (state == Qt::Checked) {
        painter->setPen(QPen(Qt::white, 2.5));

        QPainterPath checkPath;
        checkPath.moveTo(rect.left() + 5, rect.center().y());
        checkPath.lineTo(rect.left() + 9, rect.bottom() - 6);
        checkPath.lineTo(rect.right() - 5, rect.top() + 6);
        painter->drawPath(checkPath);
    }

    painter->restore();

}

QSize CheckBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), 35); // 每项高度30px
}

bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        Qt::CheckState state = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
        state = (state == Qt::Checked)? Qt::Unchecked : Qt::Checked;
        model->setData(index, state, Qt::CheckStateRole);
        return true;
    }
    return false;
}
