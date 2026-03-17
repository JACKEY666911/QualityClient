#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QAbstractButton>
#include <QColor>

class SwitchButton : public QAbstractButton
{
    Q_OBJECT
    Q_PROPERTY(QString onText READ onText WRITE setOnText)
    Q_PROPERTY(QString offText READ offText WRITE setOffText)
    Q_PROPERTY(QColor onColor READ onColor WRITE setOnColor)
    Q_PROPERTY(QColor offColor READ offColor WRITE setOffColor)
    Q_PROPERTY(QColor knobColor READ knobColor WRITE setKnobColor)
    Q_PROPERTY(QColor activeTextColor READ activeTextColor WRITE setActiveTextColor)
    Q_PROPERTY(QColor inactiveTextColor READ inactiveTextColor WRITE setInactiveTextColor)
    Q_PROPERTY(int textSpacing READ textSpacing WRITE setTextSpacing)

public:
    explicit SwitchButton(QWidget *parent = nullptr);

    QString onText() const;
    void setOnText(const QString &text);

    QString offText() const;
    void setOffText(const QString &text);

    QColor onColor() const;
    void setOnColor(const QColor &color);

    QColor offColor() const;
    void setOffColor(const QColor &color);

    QColor knobColor() const;
    void setKnobColor(const QColor &color);

    QColor activeTextColor() const;
    void setActiveTextColor(const QColor &color);

    QColor inactiveTextColor() const;
    void setInactiveTextColor(const QColor &color);

    int textSpacing() const;
    void setTextSpacing(int spacing);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_onText;
    QString m_offText;
    QColor m_onColor;
    QColor m_offColor;
    QColor m_knobColor;
    QColor m_activeTextColor;
    QColor m_inactiveTextColor;
    int m_textSpacing;
};

#endif // SWITCHBUTTON_H
