#include "SwitchButton.h"

#include <QPainter>
#include <QFontMetrics>

SwitchButton::SwitchButton(QWidget *parent)
    : QAbstractButton(parent),
      m_onText(QStringLiteral("继续")),
      m_offText(QStringLiteral("暂停")),
      m_onColor(QColor(QStringLiteral("#2ecc71"))),
      m_offColor(QColor(QStringLiteral("#808080"))),
      m_knobColor(Qt::white),
      m_activeTextColor(Qt::white),
      m_inactiveTextColor(QColor(QStringLiteral("#6c7679"))),
      m_textSpacing(6)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumHeight(32);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QString SwitchButton::onText() const
{
    return m_onText;
}

void SwitchButton::setOnText(const QString &text)
{
    if (m_onText == text) {
        return;
    }
    m_onText = text;
    update();
}

QString SwitchButton::offText() const
{
    return m_offText;
}

void SwitchButton::setOffText(const QString &text)
{
    if (m_offText == text) {
        return;
    }
    m_offText = text;
    update();
}

QColor SwitchButton::onColor() const
{
    return m_onColor;
}

void SwitchButton::setOnColor(const QColor &color)
{
    if (m_onColor == color) {
        return;
    }
    m_onColor = color;
    update();
}

QColor SwitchButton::offColor() const
{
    return m_offColor;
}

void SwitchButton::setOffColor(const QColor &color)
{
    if (m_offColor == color) {
        return;
    }
    m_offColor = color;
    update();
}

QColor SwitchButton::knobColor() const
{
    return m_knobColor;
}

void SwitchButton::setKnobColor(const QColor &color)
{
    if (m_knobColor == color) {
        return;
    }
    m_knobColor = color;
    update();
}

QColor SwitchButton::activeTextColor() const
{
    return m_activeTextColor;
}

void SwitchButton::setActiveTextColor(const QColor &color)
{
    if (m_activeTextColor == color) {
        return;
    }
    m_activeTextColor = color;
    update();
}

QColor SwitchButton::inactiveTextColor() const
{
    return m_inactiveTextColor;
}

void SwitchButton::setInactiveTextColor(const QColor &color)
{
    if (m_inactiveTextColor == color) {
        return;
    }
    m_inactiveTextColor = color;
    update();
}

int SwitchButton::textSpacing() const
{
    return m_textSpacing;
}

void SwitchButton::setTextSpacing(int spacing)
{
    if (m_textSpacing == spacing) {
        return;
    }
    m_textSpacing = spacing;
    updateGeometry();
    update();
}

QSize SwitchButton::sizeHint() const
{
    const int trackWidth = 44;
    const int trackHeight = 24;
    QFontMetrics fm(font());

    int leftTextWidth = fm.horizontalAdvance(m_offText);
    int rightTextWidth = fm.horizontalAdvance(m_onText);
    int w = leftTextWidth + rightTextWidth + trackWidth + m_textSpacing * 2 + 16;
    int h = qMax(trackHeight, fm.height()) + 8;
    return QSize(w, h);
}

void SwitchButton::setSwitchChecked(bool checked)
{
    if (isChecked() != checked) {
        setChecked(checked);
        update();  // 触发重绘
    }
}

void SwitchButton::toggle()
{
    setSwitchChecked(!isChecked());
}

void SwitchButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QFont font = this->font();
    font.setFamily("Microsoft YaHei");
    font.setPointSize(font.pointSize() + 3);
    font.setBold(true);
    painter.setFont(font);

    const int trackWidth = 44;
    const int trackHeight = 24;
    // const int margin = 2;
    QFontMetrics fm(font);
    int leftTextWidth = fm.horizontalAdvance(m_offText);
    int rightTextWidth = fm.horizontalAdvance(m_onText);
    int totalTextWidth = leftTextWidth + rightTextWidth;
    int totalWidth = totalTextWidth + trackWidth + m_textSpacing * 2;
    int startX = (width() - totalWidth) / 2;
    int centerY = height() / 2;

    QRectF trackRect(startX + leftTextWidth + m_textSpacing,
                     centerY - trackHeight / 2,
                     trackWidth,
                     trackHeight);
    qreal radius = trackRect.height() / 2.0;

    QColor trackColor = isChecked() ? m_onColor : m_offColor;
    painter.setPen(Qt::NoPen);
    painter.setBrush(trackColor);
    painter.drawRoundedRect(trackRect, radius, radius);

    qreal knobDiameter = trackRect.height() - 2;
    qreal knobY = trackRect.top() + 1;
    qreal knobX = isChecked() ? (trackRect.right() - knobDiameter - 1)
                              : (trackRect.left() + 1);
    QRectF knobRect(knobX, knobY, knobDiameter, knobDiameter);

    painter.setBrush(m_knobColor);
    painter.drawEllipse(knobRect);

    QRect leftTextRect(startX, 0, leftTextWidth, height());
    QRect rightTextRect(trackRect.right() + m_textSpacing,
                        0,
                        rightTextWidth,
                        height());

    painter.setPen(isChecked() ? m_inactiveTextColor : m_activeTextColor);
    painter.drawText(leftTextRect, Qt::AlignVCenter | Qt::AlignLeft, m_offText);

    painter.setPen(isChecked() ? m_activeTextColor : m_inactiveTextColor);
    painter.drawText(rightTextRect, Qt::AlignVCenter | Qt::AlignLeft, m_onText);
}
