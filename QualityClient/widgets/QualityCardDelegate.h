#ifndef QUALITYCARDDELEGATE_H
#define QUALITYCARDDELEGATE_H

#include <QHash>
#include <QList>
#include <QPainter>
#include <QPersistentModelIndex>
#include <QStyledItemDelegate>

class QualityCardDelegate : public QStyledItemDelegate {
  Q_OBJECT
 public:
  explicit QualityCardDelegate(QObject* parent = nullptr);

  // 基础重写
  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

 protected:
  void renderImage(QPainter* painter, const QRect& rect,
                   const QPixmap& pix) const {
    if (pix.isNull()) return;
    int x = rect.left() + (rect.width() - pix.width()) / 2;
    int y = rect.top() + (rect.height() - pix.height()) / 2;
    painter->drawPixmap(x, y, pix);
  }

 private:
  // 绘图辅助
  void drawTag(QPainter* painter, int x, int y, int w, int h,
               const QString& text, const QColor& color) const;
  void trackPendingUrl(const QString& url, const QModelIndex& index) const;

  // 逻辑支撑函数 (根据你的需求保留)
  QColor getColor(int res) const;
  QString getChannelText(int res) const;
  QString getQualityText(int res) const;

  // 异步加载辅助：记录哪些 Index 正在等待哪张 URL 的下载
  mutable QHash<QString, QList<QPersistentModelIndex>> m_pendingIndexes;
  void drawAsyncImage(QPainter* painter, const QRect& rect, const QString& url,
                      const QModelIndex& index, const QString& viewTag) const;
};

#endif  // QUALITYCARDDELEGATE_H
