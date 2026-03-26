#ifndef TOASTNOTIFICATION_H
#define TOASTNOTIFICATION_H

#include <QFrame>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class ToastNotification : public QFrame
{
    Q_OBJECT
public:
    enum Type { Info, Success, Warning, Error };
    explicit ToastNotification(QWidget *parent = nullptr);

    static void showToast(QWidget *parent, const QString &message,
                          Type type = Info, int durationMs = 3000);

protected:
    void applyTypeStyle(Type type);

private:
    QLabel *m_iconLabel;
    QLabel *m_messageLabel;
    QGraphicsDropShadowEffect *m_shadow;
};

#endif