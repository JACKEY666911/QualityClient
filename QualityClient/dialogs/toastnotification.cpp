#include "toastnotification.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTimer>

ToastNotification::ToastNotification(QWidget* parent)
    : QFrame(parent),
      m_iconLabel(new QLabel(this)),
      m_messageLabel(new QLabel(this)) {
  // 1. 窗口属性
  setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName(QStringLiteral("toastNotification"));

  // 2. 布局设计：超大间距提升高级感
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout->setContentsMargins(40, 14, 40, 14);
  layout->setSpacing(15);  // 图标和文字间距稍微收紧一点
  setMinimumWidth(320);
  // 图标加大
  m_iconLabel->setFixedSize(26, 26);  // 稍微缩小1px配合扁平化
  m_iconLabel->setAlignment(Qt::AlignCenter);

  // 文字
  m_messageLabel->setStyleSheet(
      "font-family: 'Segoe UI', 'Microsoft YaHei UI';"
      "font-size: 18px;"
      "font-weight: 600;"
      "color: #FFFFFF;");
  m_messageLabel->setWordWrap(false);

  layout->addWidget(m_iconLabel);
  layout->addWidget(m_messageLabel);
}

void ToastNotification::showToast(QWidget* parent, const QString& message,
                                  Type type, int durationMs) {
  QWidget* anchor = parent ? parent->window() : QApplication::activeWindow();
  ToastNotification* toast = new ToastNotification(anchor);

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

  QParallelAnimationGroup* enterGroup = new QParallelAnimationGroup(toast);

  QPropertyAnimation* posAnim = new QPropertyAnimation(toast, "pos");
  posAnim->setDuration(600);
  posAnim->setStartValue(toast->pos());
  posAnim->setEndValue(targetPos);
  posAnim->setEasingCurve(QEasingCurve::OutBack);

  QPropertyAnimation* opacityAnim =
      new QPropertyAnimation(toast, "windowOpacity");
  opacityAnim->setDuration(400);
  opacityAnim->setStartValue(0.0);
  opacityAnim->setEndValue(1.0);

  enterGroup->addAnimation(posAnim);
  enterGroup->addAnimation(opacityAnim);
  enterGroup->start(QAbstractAnimation::DeleteWhenStopped);

  QTimer::singleShot(durationMs, toast, [toast]() {
    if (!toast) return;
    QPropertyAnimation* fadeOut =
        new QPropertyAnimation(toast, "windowOpacity");
    fadeOut->setDuration(500);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    connect(fadeOut, &QPropertyAnimation::finished, toast,
            &ToastNotification::close);
    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
  });
}

void ToastNotification::applyTypeStyle(Type type) {
  QString accentColor;
  QString iconText;

  // 采用明度较高的饱和色，确保在深色背景下耀眼
  switch (type) {
    case Success:
      accentColor = "#52C41A";
      iconText = "✔";
      break;
    case Error:
      accentColor = "#FF4D4F";
      iconText = "✘";
      break;
    case Warning:
      accentColor = "#FAAD14";
      iconText = "⚠";
      break;
    case Info:
      accentColor = "#1890FF";
      iconText = "ℹ";
      break;
  }

  m_iconLabel->setText(iconText);
  m_iconLabel->setStyleSheet(QString("color: %1; font-size: 26px; font-weight: "
                                     "bold; background: transparent;")
                                 .arg(accentColor));
}

void ToastNotification::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);

  // 必须开启抗锯齿
  painter.setRenderHint(QPainter::Antialiasing);

  // 显式设置无描边，防止边缘出现 1px 的杂色细线
  painter.setPen(Qt::NoPen);

  // 定义填充色
  painter.setBrush(QBrush(QColor(40, 44, 52, 245)));

  // 使用 QPainterPath 构建绘制路径
  QPainterPath path;
  path.addRoundedRect(this->rect(), 15, 15);

  // 将路径填充到画布上，完美圆角，绝对没有直角蒙版问题
  painter.fillPath(path, painter.brush());
}
