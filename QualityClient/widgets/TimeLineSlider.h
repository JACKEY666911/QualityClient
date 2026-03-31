#ifndef TIMELINESLIDER_H
#define TIMELINESLIDER_H

#include <QAbstractSlider>
#include <QWidget>

struct TimeLineNode {
  double position; // 归一化位置 [0.0, 1.0]
  QString label;   // 显示文字
  int timestamp;   // 对应时间戳（秒），用于气泡计算

  TimeLineNode(double pos = 0.0, const QString &lbl = "", int ts = 0)
      : position(qBound(0.0, pos, 1.0)), label(lbl), timestamp(ts) {}
};

class TimeLineSlider : public QAbstractSlider {
  Q_OBJECT
public:
  explicit TimeLineSlider(QWidget *parent = nullptr);

  void clearNodes();
  void addNode(double position, const QString &label, int timestampSec = 0);
  void setNodes(const QList<TimeLineNode> &nodes);
  QList<TimeLineNode> nodes() const { return m_nodes; }

  // 外观配置
  void setTrackHeight(int h);
  void setNodeRadius(int r);
  void setLabelFontSize(int size);
  void setBubbleEnabled(bool on);
  void setSnapToNode(bool on);

  // 颜色配置
  void setTrackColor(const QColor &c);
  void setProgressColor(const QColor &c);
  void setNodeActiveColor(const QColor &c);
  void setNodeInactiveColor(const QColor &c);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;

signals:

private:
  void drawTrack(QPainter &p, const QRectF &rect);
  void drawProgress(QPainter &p, const QRectF &rect, double ratio);
  void drawNodes(QPainter &p, const QRectF &rect, double ratio);
  void drawHandle(QPainter &p, const QRectF &rect, double ratio);
  void drawLabels(QPainter &p, const QRectF &rect);
  void drawBubble(QPainter &p, const QPoint &anchor, const QString &text);
  QString timeStringFromX(int x) const;
  int findSnapNode(double ratio) const;

  // 数据
  QList<TimeLineNode> m_nodes;

  // 交互状态
  bool m_showTip = false;
  bool m_isDragging = false;
  bool m_bubbleEnabled = true;
  bool m_snapToNode = false;
  QPoint m_hoverPos;

  // 外观参数
  int m_trackHeight = 6;
  int m_nodeRadius = 5;
  int m_handleRadius = 8;
  int m_labelFontSize = 8;
  int m_bubbleFontSize = 9;

  // 颜色
  QColor m_trackColor = QColor("#E0E0E0");
  QColor m_progressColor = QColor("#3BA0FF");
  QColor m_nodeActiveColor = QColor("#3BA0FF");
  QColor m_nodeInactiveColor = QColor("#8C8C8C");
};

#endif // TIMELINESLIDER_H
