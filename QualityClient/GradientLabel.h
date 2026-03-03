#ifndef GRADIENTLABEL_H
#define GRADIENTLABEL_H

#include <QLabel>
#include <QColor>
#include <QLinearGradient>
#include <QPointF>
#include <QMetaType>

// 保留所有渐变方向枚举
enum class GradientDirection {
    Vertical,    // 垂直（中心顶→中心底，匹配WPF效果）
    Horizontal,  // 水平（左→右）
    Diagonal,    // 对角线（左上→右下）
    Custom       // 自定义起始/结束点
};
Q_DECLARE_METATYPE(GradientDirection)

class GradientLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(GradientDirection gradientDirection READ gradientDirection WRITE setGradientDirection NOTIFY gradientDirectionChanged)
    Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
    Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)

public:
    explicit GradientLabel(QWidget *parent = nullptr);

    QColor startColor() const;
    void setStartColor(const QColor &color);

    QColor endColor() const;
    void setEndColor(const QColor &color);

    GradientDirection gradientDirection() const;
    void setGradientDirection(GradientDirection direction);

    void setCustomGradientPoints(const QPointF &startPoint, const QPointF &endPoint);

signals:
    void startColorChanged();
    void endColorChanged();
    void gradientDirectionChanged();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initDefaultSettings();
    QLinearGradient createGradient();

    // 仅保留核心成员变量
    QColor m_startColor;
    QColor m_endColor;
    GradientDirection m_direction;
    QPointF m_customStartPoint;
    QPointF m_customEndPoint;
};

#endif // GRADIENTLABEL_H
