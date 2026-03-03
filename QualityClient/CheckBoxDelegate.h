#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QWidget>
#include <QStyledItemDelegate>
class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CheckBoxDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    void drawCustomCheckBox(QPainter *painter, const QRect &rect, Qt::CheckState state, bool hover) const;

signals:

};

#endif // CHECKBOXDELEGATE_H
