#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include <QWidget>

class QPixmap;

class ThumbnailWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ThumbnailWidget(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);
    void setViewRectNormalized(const QRectF &rect);

signals:
    void clickedAtNormalized(qreal xRatio, qreal yRatio);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QRectF imageRect() const;

    QPixmap m_pixmap;
    QRectF m_viewRectNormalized;
};

#endif // THUMBNAILWIDGET_H
