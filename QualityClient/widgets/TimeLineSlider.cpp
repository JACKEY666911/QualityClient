#include "TimeLineSlider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>

TimeLineSlider::TimeLineSlider(QWidget *parent) : QAbstractSlider{parent} {
  setMouseTracking(true);
  setFixedHeight(70); // 上方气泡 + 下方标签
  setRange(0, 10000); // 高精度范围
  setCursor(Qt::PointingHandCursor);
}

void TimeLineSlider::clearNodes() {
  m_nodes.clear();
  update();
}

void TimeLineSlider::addNode(double position, const QString &label,
                             int timestampSec) {
  m_nodes.append(TimeLineNode(position, label, timestampSec));
  std::sort(m_nodes.begin(), m_nodes.end(),
            [](const TimeLineNode &a, const TimeLineNode &b) {
              return a.position < b.position;
            });
  update();
}

void TimeLineSlider::setNodes(const QList<TimeLineNode> &nodes) {
  m_nodes = nodes;
  std::sort(m_nodes.begin(), m_nodes.end(),
            [](const TimeLineNode &a, const TimeLineNode &b) {
              return a.position < b.position;
            });
  update();
}

// 外观配置
void TimeLineSlider::setTrackHeight(int h) {
  m_trackHeight = h;
  update();
}
void TimeLineSlider::setNodeRadius(int r) {
  m_nodeRadius = r;
  update();
}
void TimeLineSlider::setLabelFontSize(int s) {
  m_labelFontSize = s;
  update();
}
void TimeLineSlider::setBubbleEnabled(bool on) {
  m_bubbleEnabled = on;
  update();
}
void TimeLineSlider::setSnapToNode(bool on) { m_snapToNode = on; }

void TimeLineSlider::setTrackColor(const QColor &c) {
  m_trackColor = c;
  update();
}
void TimeLineSlider::setProgressColor(const QColor &c) {
  m_progressColor = c;
  update();
}
void TimeLineSlider::setNodeActiveColor(const QColor &c) {
  m_nodeActiveColor = c;
  update();
}
void TimeLineSlider::setNodeInactiveColor(const QColor &c) {
  m_nodeInactiveColor = c;
  update();
}

// 辅助：根据鼠标X计算时间字符串
QString TimeLineSlider::timeStringFromX(int x) const {
  if (width() <= 0)
    return "00:00:00";

  double ratio = qBound(0.0, static_cast<double>(x) / width(), 1.0);
  int totalSec = qRound(ratio * (maximum() - minimum()) + minimum());

  // 尝试找到最近的节点，使用其精确时间戳
  int bestDist = INT_MAX;
  int nodeTime = -1;
  for (const auto &node : m_nodes) {
    int dist = qAbs(static_cast<int>(node.position * width()) - x);
    if (dist < bestDist) {
      bestDist = dist;
      nodeTime = node.timestamp;
    }
  }
  if (bestDist < 20 && nodeTime >= 0) {
    totalSec = nodeTime;
  }

  int h = totalSec / 3600;
  int m = (totalSec % 3600) / 60;
  int s = totalSec % 60;
  return QString("%1:%2:%3")
      .arg(h, 2, 10, QChar('0'))
      .arg(m, 2, 10, QChar('0'))
      .arg(s, 2, 10, QChar('0'));
}

// 辅助：查找最近节点（用于吸附）
int TimeLineSlider::findSnapNode(double ratio) const {
  const int snapThreshold = 15; // 像素
  int bestIdx = -1;
  int bestDist = snapThreshold + 1;

  for (int i = 0; i < m_nodes.size(); ++i) {
    int nodePixelX = static_cast<int>(m_nodes[i].position * width());
    int cursorPixelX = static_cast<int>(ratio * width());
    int dist = qAbs(nodePixelX - cursorPixelX);
    if (dist < bestDist) {
      bestDist = dist;
      bestIdx = i;
    }
  }
  return bestDist <= snapThreshold ? bestIdx : -1;
}

static QRectF trackRect(int w, int trackY, int trackH) {
  return QRectF(0, trackY - trackH / 2.0, w, trackH);
}

// 绘制 — 背景轨道
void TimeLineSlider::drawTrack(QPainter &p, const QRectF &rect) {
  // 轨道底色
  p.setPen(Qt::NoPen);
  p.setBrush(m_trackColor);
  p.drawRoundedRect(rect, rect.height() / 2.0, rect.height() / 2.0);

  // 微弱内凹阴影
  QLinearGradient shadow(rect.topLeft(), rect.bottomLeft());
  shadow.setColorAt(0.0, QColor(0, 0, 0, 12));
  shadow.setColorAt(0.5, QColor(0, 0, 0, 0));
  shadow.setColorAt(1.0, QColor(0, 0, 0, 8));
  p.setBrush(shadow);
  p.drawRoundedRect(rect, rect.height() / 2.0, rect.height() / 2.0);
}

// 绘制 — 已播放进度（裁剪到轨道形状内）
void TimeLineSlider::drawProgress(QPainter &p, const QRectF &rect,
                                  double ratio) {
  if (ratio <= 0.001)
    return;

  // 关键修复：用 clipPath 裁剪，避免独立圆角
  QPainterPath clipPath;
  clipPath.addRoundedRect(rect, rect.height() / 2.0, rect.height() / 2.0);
  p.save();
  p.setClipPath(clipPath);

  double pw = rect.width() * ratio;
  QRectF progRect(rect.x(), rect.y(), pw, rect.height());

  // 渐变色
  QLinearGradient grad(rect.topLeft(), rect.topRight());
  grad.setColorAt(0.0, m_progressColor);
  grad.setColorAt(1.0, m_progressColor.lighter(115));
  p.setBrush(grad);
  p.drawRoundedRect(progRect, rect.height() / 2.0, rect.height() / 2.0);

  // 顶部高光条
  QRectF hlRect(rect.x(), rect.y(), pw, rect.height() * 0.4);
  QLinearGradient hlGrad(rect.topLeft(), rect.bottomLeft());
  hlGrad.setColorAt(0.0, QColor(255, 255, 255, 60));
  hlGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
  p.setBrush(hlGrad);
  p.drawRoundedRect(hlRect, rect.height() / 2.0, rect.height() / 2.0);

  p.restore();
}

// 绘制 — 节点圆点
void TimeLineSlider::drawNodes(QPainter &p, const QRectF &rect, double ratio) {
  for (const auto &node : m_nodes) {
    double nx = rect.x() + rect.width() * node.position;
    double ny = rect.y() + rect.height() / 2.0;
    bool isActive = (node.position <= ratio + 0.001);

    QColor fillColor = isActive ? m_nodeActiveColor : m_nodeInactiveColor;

    // 外圈白边
    p.setPen(QPen(Qt::white, 2));
    p.setBrush(fillColor);
    p.drawEllipse(QPointF(nx, ny), m_nodeRadius, m_nodeRadius);

    // 已激活节点：内部小亮点
    if (isActive) {
      QRadialGradient glow(nx - m_nodeRadius * 0.25, ny - m_nodeRadius * 0.25,
                           m_nodeRadius * 0.5);
      glow.setColorAt(0.0, QColor(255, 255, 255, 120));
      glow.setColorAt(1.0, QColor(255, 255, 255, 0));
      p.setPen(Qt::NoPen);
      p.setBrush(glow);
      p.drawEllipse(QPointF(nx, ny), m_nodeRadius - 1.5, m_nodeRadius - 1.5);
    }
  }
}

// 绘制 — 拖拽手柄（当前位置大圆点）
void TimeLineSlider::drawHandle(QPainter &p, const QRectF &rect, double ratio) {
  double hx = rect.x() + rect.width() * ratio;
  double hy = rect.y() + rect.height() / 2.0;
  int r = m_handleRadius;

  // 外发光
  QRadialGradient outerGlow(hx, hy, r + 6);
  outerGlow.setColorAt(0.0,
                       QColor(m_progressColor.red(), m_progressColor.green(),
                              m_progressColor.blue(), 50));
  outerGlow.setColorAt(1.0,
                       QColor(m_progressColor.red(), m_progressColor.green(),
                              m_progressColor.blue(), 0));
  p.setPen(Qt::NoPen);
  p.setBrush(outerGlow);
  p.drawEllipse(QPointF(hx, hy), r + 6, r + 6);

  // 手柄本体
  p.setPen(QPen(Qt::white, 2.5));
  p.setBrush(m_progressColor);
  p.drawEllipse(QPointF(hx, hy), r, r);

  // 手柄内部高光
  QRadialGradient inner(hx - r * 0.3, hy - r * 0.3, r * 0.5);
  inner.setColorAt(0.0, QColor(255, 255, 255, 140));
  inner.setColorAt(1.0, QColor(255, 255, 255, 0));
  p.setPen(Qt::NoPen);
  p.setBrush(inner);
  p.drawEllipse(QPointF(hx, hy), r - 2, r - 2);
}

// 绘制 — 节点下方标签
void TimeLineSlider::drawLabels(QPainter &p, const QRectF &rect) {
  if (m_labelFontSize <= 0)
    return;

  QFont font("Microsoft YaHei", m_labelFontSize);
  p.setFont(font);
  p.setPen(QColor("#666666"));

  QFontMetrics fm(font);
  int labelY = static_cast<int>(rect.bottom() + 6);

  for (const auto &node : m_nodes) {
    double nx = rect.x() + rect.width() * node.position;
    int textW = fm.horizontalAdvance(node.label);
    int halfW = textW / 2;

    // 靠近边缘时调整对齐
    Qt::Alignment align = Qt::AlignHCenter;
    if (nx - halfW < 0)
      align = Qt::AlignLeft;
    else if (nx + halfW > width())
      align = Qt::AlignRight;

    p.drawText(QRectF(0, labelY, width(), fm.height()), align, node.label);
  }
}

// 绘制 — 悬浮气泡
void TimeLineSlider::drawBubble(QPainter &p, const QPoint &anchor,
                                const QString &text) {
  p.save();
  QFont font("Microsoft YaHei", m_bubbleFontSize);
  p.setFont(font);

  QFontMetrics fm(font);
  QRect textRect = fm.boundingRect(text);
  textRect.adjust(-12, -6, 12, 6);

  // 气泡位置：锚点正上方
  QRect bubbleRect;
  bubbleRect.setSize(textRect.size());
  bubbleRect.moveCenter(
      QPoint(anchor.x(), anchor.y() - textRect.height() / 2 - 14));

  // 边缘保护
  if (bubbleRect.left() < 2)
    bubbleRect.moveLeft(2);
  if (bubbleRect.right() > width() - 2)
    bubbleRect.moveRight(width() - 2);
  if (bubbleRect.top() < 0)
    bubbleRect.moveTop(0);

  // 气泡背景
  QColor bubbleBg(50, 50, 60, 230);
  p.setBrush(bubbleBg);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(bubbleRect, 6, 6);

  // 倒三角（尖端对准锚点）
  int triX = qBound(bubbleRect.left() + 8, anchor.x(), bubbleRect.right() - 8);
  QPainterPath triPath;
  triPath.moveTo(triX - 6, bubbleRect.bottom());
  triPath.lineTo(triX + 6, bubbleRect.bottom());
  triPath.lineTo(triX, bubbleRect.bottom() + 7);
  triPath.closeSubpath();
  p.fillPath(triPath, bubbleBg);

  // 文字
  p.setPen(Qt::white);
  p.drawText(bubbleRect, Qt::AlignCenter, text);
  p.restore();
}

// 主绘制
void TimeLineSlider::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event)
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  int w = width();
  int trackY = height() - 28; // 给下方标签留空间
  QRectF tRect = trackRect(w, trackY, m_trackHeight);

  double ratio =
      static_cast<double>(value() - minimum()) / qMax(1, maximum() - minimum());

  drawTrack(p, tRect);
  drawProgress(p, tRect, ratio);
  drawNodes(p, tRect, ratio);
  drawHandle(p, tRect, ratio);
  drawLabels(p, tRect);

  // 悬浮气泡
  if (m_showTip && m_bubbleEnabled) {
    QString timeText = timeStringFromX(m_hoverPos.x());
    drawBubble(p, QPoint(m_hoverPos.x(), trackY - m_trackHeight / 2), timeText);
  }
}

// 鼠标事件
void TimeLineSlider::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton || width() <= 0)
    return;

  m_isDragging = true;
  double pos = static_cast<double>(event->pos().x()) / width();
  pos = qBound(0.0, pos, 1.0);

  // 吸附到节点
  if (m_snapToNode) {
    int snapIdx = findSnapNode(pos);
    if (snapIdx >= 0) {
      pos = m_nodes[snapIdx].position;
    }
  }

  int newVal = static_cast<int>(minimum() + pos * (maximum() - minimum()));
  setValue(newVal);
  emit sliderMoved(value());
}

void TimeLineSlider::mouseMoveEvent(QMouseEvent *event) {
  m_hoverPos = event->pos();

  if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
    double pos = static_cast<double>(event->pos().x()) / width();
    pos = qBound(0.0, pos, 1.0);

    if (m_snapToNode) {
      int snapIdx = findSnapNode(pos);
      if (snapIdx >= 0)
        pos = m_nodes[snapIdx].position;
    }

    int newVal = static_cast<int>(minimum() + pos * (maximum() - minimum()));
    setValue(newVal);
    emit sliderMoved(value());
  }

  update(); // 刷新气泡位置
}

void TimeLineSlider::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_isDragging = false;
  }
}

void TimeLineSlider::enterEvent(QEnterEvent *event) {
  Q_UNUSED(event)
  m_showTip = true;
  update();
}

void TimeLineSlider::leaveEvent(QEvent *event) {
  Q_UNUSED(event)
  m_showTip = false;
  m_isDragging = false;
  update();
}