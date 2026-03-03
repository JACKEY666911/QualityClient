#include "GradientLabel.h"
#include <QPainter>
#include <QFont>
#include <QRectF>

GradientLabel::GradientLabel(QWidget *parent)
    : QLabel(parent)
{
    initDefaultSettings();
}

void GradientLabel::initDefaultSettings()
{
    // 默认渐变参数（匹配WPF垂直渐变）
    m_startColor = Qt::white;
    m_endColor = QColor("#00E1FF");
    m_direction = GradientDirection::Vertical;
    m_customStartPoint = QPointF(0.5, 0);
    m_customEndPoint = QPointF(0.5, 1);

    // 文字居中（QLabel原生属性）
    setAlignment(Qt::AlignCenter);
}

QColor GradientLabel::startColor() const
{
    return m_startColor;
}

void GradientLabel::setStartColor(const QColor &color)
{
    if (m_startColor != color) {
        m_startColor = color;
        emit startColorChanged();
        update();
    }
}

QColor GradientLabel::endColor() const
{
    return m_endColor;
}

void GradientLabel::setEndColor(const QColor &color)
{
    if (m_endColor != color) {
        m_endColor = color;
        emit endColorChanged();
        update();
    }
}

GradientDirection GradientLabel::gradientDirection() const
{
    return m_direction;
}

void GradientLabel::setGradientDirection(GradientDirection direction)
{
    if (m_direction != direction) {
        m_direction = direction;
        emit gradientDirectionChanged();
        update();
    }
}

void GradientLabel::setCustomGradientPoints(const QPointF &startPoint, const QPointF &endPoint)
{
    m_customStartPoint = startPoint;
    m_customEndPoint = endPoint;
    m_direction = GradientDirection::Custom;
    update();
}

QLinearGradient GradientLabel::createGradient()
{
    QRectF rect = this->rect();
    QLinearGradient gradient;

    // 保留所有渐变方向的绘制逻辑
    switch (m_direction) {
    case GradientDirection::Vertical:
        gradient = QLinearGradient(rect.center().x(), rect.top(),
                                   rect.center().x(), rect.bottom());
        break;
    case GradientDirection::Horizontal:
        gradient = QLinearGradient(rect.left(), rect.center().y(),
                                   rect.right(), rect.center().y());
        break;
    case GradientDirection::Diagonal:
        gradient = QLinearGradient(rect.topLeft(), rect.bottomRight());
        break;
    case GradientDirection::Custom:
        gradient = QLinearGradient(
            rect.width() * m_customStartPoint.x(), rect.height() * m_customStartPoint.y(),
            rect.width() * m_customEndPoint.x(), rect.height() * m_customEndPoint.y()
        );
        break;
    }

    gradient.setColorAt(0.0, m_startColor);
    gradient.setColorAt(1.0, m_endColor);
    gradient.setSpread(QGradient::PadSpread);

    return gradient;
}

void GradientLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    // 抗锯齿+平滑渐变，保证视觉效果
    painter.setRenderHints(QPainter::Antialiasing |
                           QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform);

    // 创建对应方向的渐变
    QLinearGradient gradient = createGradient();

    // 使用QLabel原生字体（字号/加粗通过原生font属性设置）
    painter.setPen(QPen(gradient,1));
    painter.setFont(this->font());

    // 绘制渐变文字
    painter.drawText(rect(), alignment(), text());
}
