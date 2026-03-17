#include "ThumbnailWidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

ThumbnailWidget::ThumbnailWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(160, 90);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void ThumbnailWidget::setImage(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

void ThumbnailWidget::setViewRectNormalized(const QRectF &rect)
{
    m_viewRectNormalized = rect;
    update();
}

void ThumbnailWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.fillRect(rect(), QColor(QStringLiteral("#d9d9d9")));

    if (m_pixmap.isNull()) {
        painter.setPen(QColor(QStringLiteral("#666666")));
//        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("缩略图"));
        return;
    }

    QRectF target = imageRect();
    painter.drawPixmap(target.toRect(), m_pixmap);

    if (!m_viewRectNormalized.isNull()) {
        QRectF viewRect(
            target.left() + m_viewRectNormalized.left() * target.width(),
            target.top() + m_viewRectNormalized.top() * target.height(),
            m_viewRectNormalized.width() * target.width(),
            m_viewRectNormalized.height() * target.height()
        );
        QPen pen(QColor(QStringLiteral("#ff0000")));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(viewRect);
    }
}

void ThumbnailWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(event);
        return;
    }
    if (m_pixmap.isNull()) {
        return;
    }
    QRectF target = imageRect();
    if (!target.contains(event->pos())) {
        return;
    }
    qreal xRatio = (event->pos().x() - target.left()) / target.width();
    qreal yRatio = (event->pos().y() - target.top()) / target.height();
    emit clickedAtNormalized(xRatio, yRatio);
}

QRectF ThumbnailWidget::imageRect() const
{
    if (m_pixmap.isNull()) {
        return QRectF();
    }
    QSizeF viewSize = size();
    QSizeF imgSize = m_pixmap.size();
    qreal scale = qMin(viewSize.width() / imgSize.width(), viewSize.height() / imgSize.height());
    QSizeF scaled = imgSize * scale;
    QPointF topLeft((viewSize.width() - scaled.width()) / 2.0,
                    (viewSize.height() - scaled.height()) / 2.0);
    return QRectF(topLeft, scaled);
}
