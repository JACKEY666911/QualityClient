#include "toastnotification.h"
#include <QHBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QScreen>

ToastNotification::ToastNotification(QWidget *parent)
    : QFrame(parent)
    , m_iconLabel(new QLabel(this))
    , m_messageLabel(new QLabel(this))
{
    // 1. 窗口属性
    setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("toastNotification"));

    // 2. 布局设计：超大间距提升高级感
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(35, 22, 35, 22);
    layout->setSpacing(20);

    // 图标加大
    m_iconLabel->setFixedSize(30, 30);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    // 文字：使用纯白或近白色，在深色背景下对比度最高
    m_messageLabel->setStyleSheet(
        "font-family: 'Segoe UI', 'Microsoft YaHei UI';"
        "font-size: 18px;"
        "font-weight: 600;"
        "color: #FFFFFF;"
        );
    m_messageLabel->setWordWrap(false);

    layout->addWidget(m_iconLabel);
    layout->addWidget(m_messageLabel);

    // 3. 增强投影：由于背景是深色的，投影需要更深一些才能拉开层次
    m_shadow = new QGraphicsDropShadowEffect(this);
    m_shadow->setBlurRadius(45);
    m_shadow->setColor(QColor(0, 0, 0, 100)); // 较重的黑色阴影
    m_shadow->setOffset(0, 10);
    setGraphicsEffect(m_shadow);
}

void ToastNotification::showToast(QWidget *parent, const QString &message, Type type, int durationMs)
{
    QWidget *anchor = parent ? parent->window() : QApplication::activeWindow();
    ToastNotification *toast = new ToastNotification(anchor);

    toast->m_messageLabel->setText(message);
    toast->applyTypeStyle(type);
    toast->adjustSize();

    // 4. 位置：固定在窗口上半部分的黄金位置
    QPoint targetPos;
    if (anchor) {
        QRect rect = anchor->geometry();
        targetPos = QPoint(rect.center().x() - toast->width() / 2, rect.top() + 80);
    } else {
        QRect screenRect = QApplication::primaryScreen()->geometry();
        targetPos = QPoint(screenRect.center().x() - toast->width() / 2, 150);
    }

    // 5. 进场动效：从上方飘落并弹跳
    toast->move(targetPos - QPoint(0, 50));
    toast->setWindowOpacity(0.0);
    toast->show();

    QParallelAnimationGroup *enterGroup = new QParallelAnimationGroup(toast);

    QPropertyAnimation *posAnim = new QPropertyAnimation(toast, "pos");
    posAnim->setDuration(600);
    posAnim->setStartValue(toast->pos());
    posAnim->setEndValue(targetPos);
    posAnim->setEasingCurve(QEasingCurve::OutBack);

    QPropertyAnimation *opacityAnim = new QPropertyAnimation(toast, "windowOpacity");
    opacityAnim->setDuration(400);
    opacityAnim->setStartValue(0.0);
    opacityAnim->setEndValue(1.0);

    enterGroup->addAnimation(posAnim);
    enterGroup->addAnimation(opacityAnim);
    enterGroup->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer::singleShot(durationMs, toast, [toast]() {
        if (!toast) return;
        QPropertyAnimation *fadeOut = new QPropertyAnimation(toast, "windowOpacity");
        fadeOut->setDuration(500);
        fadeOut->setStartValue(1.0);
        fadeOut->setEndValue(0.0);
        connect(fadeOut, &QPropertyAnimation::finished, toast, &ToastNotification::close);
        fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
    });
}

void ToastNotification::applyTypeStyle(Type type)
{
    QString accentColor;
    QString iconText;

    // 采用明度较高的饱和色，确保在深色背景下耀眼
    switch (type) {
    case Success: accentColor = "#52C41A"; iconText = "✔"; break;
    case Error:   accentColor = "#FF4D4F"; iconText = "✘"; break;
    case Warning: accentColor = "#FAAD14"; iconText = "⚠"; break;
    case Info:    accentColor = "#1890FF"; iconText = "ℹ"; break;
    }

    m_iconLabel->setText(iconText);
    m_iconLabel->setStyleSheet(QString("color: %1; font-size: 26px; font-weight: bold;").arg(accentColor));

    // --- 样式表：深色大圆角矩形 ---
    setStyleSheet(QString(
                      "QFrame#toastNotification {"
                      "  background-color: rgba(40, 44, 52, 245);" // 深色磨砂质感 (Dark Slate)
                      "  border: 1px solid rgba(255, 255, 255, 0.15);" // 极细的高亮边框（增加精致感）
                      "  border-radius: 20px;"                       // 巨大的圆角，非常现代
                      "  padding-left: 10px;"
                      "}"
                      ).arg(accentColor));
}
