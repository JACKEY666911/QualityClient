#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QWidget>
#include <QScopedPointer>
class FlowLayoutPrivate;

class FlowLayout : public QLayout
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FlowLayout)
public:
    explicit FlowLayout(QWidget *parent = nullptr,int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();
    void setCenterEnable(bool enable);
    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;
    int horizontalSpacing();
    int verticalSpacing();
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;

signals:
    void layoutMarginChange(int);

private:
    QList<QLayoutItem*> m_itemList;
    QScopedPointer<FlowLayoutPrivate> d_ptr;
};

#endif // FLOWLAYOUT_H
