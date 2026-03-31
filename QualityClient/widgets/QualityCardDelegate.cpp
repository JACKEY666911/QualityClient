#include "QualityCardDelegate.h"

#include <services/imageprovider.h>

#include <QAbstractItemView>
#include <QPixmapCache>
QualityCardDelegate::QualityCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent) {
  // 监听图片加载器的信号
  connect(&ImageProvider::instance(), &ImageProvider::imageUpdated, this,
          [this](const QString &url, bool success) {
            // 1. 检查该 URL 是否在我们的等待队列中
            if (!m_pendingIndexes.contains(url))
              return;

            // 2.取出所有受影响的索引
            auto indexes = m_pendingIndexes.value(url);

            // 3.无论成功还是失败，都从等待队列中移除
            // 这样如果失败了，下次 paint 时可以再次发起 request，而不是永远卡在
            // pending 状态
            m_pendingIndexes.remove(url);

            if (!success) {
              qDebug() << "Image load failed for URL:" << url;
              return;
            }

            // 4. 只有成功时才通知 View 重绘
            for (const auto &pIndex : indexes) {
              if (pIndex.isValid()) {
                auto *model = const_cast<QAbstractItemModel *>(pIndex.model());
                // 发送信号告知该项数据已变（DecorationRole 通常代表图标/图片）
                emit model->dataChanged(pIndex, pIndex, {Qt::DecorationRole});
              }
            }
          });
}

QColor QualityCardDelegate::getColor(int res) const {
  // 1 代表正常/放行(绿色)，其他代表异常(红色/橙色)
  return (res == 1) ? QColor("#7ED321") : QColor("#FF4D4F");
}

QString QualityCardDelegate::getChannelText(int res) const {
  return (res == 1) ? "通道放行" : "通道截留";
}

QString QualityCardDelegate::getQualityText(int res) const {
  return (res == 1) ? "质控放行" : "质控异常";
}

void QualityCardDelegate::drawAsyncImage(QPainter *painter, const QRect &rect,
                                         const QString &url,
                                         const QModelIndex &index,
                                         const QString &viewTag) const {
  if (url.isEmpty())
    return;

  // 缓存键包含 viewTag 以区分同一 Index 下的不同视角
  QString cacheKey = QString("thumb_%1_%2_%3_%4")
                         .arg(url)
                         .arg(rect.width())
                         .arg(rect.height())
                         .arg(viewTag);

  QPixmap scaledPix;
  if (!QPixmapCache::find(cacheKey, &scaledPix)) {
    QPixmap originalPix = ImageProvider::instance().cached(url);

    if (originalPix.isNull()) {
      // 注册等待队列并请求
      trackPendingUrl(url, index);
      ImageProvider::instance().request(url);

      // 绘制占位图
      painter->setBrush(QColor("#F5F5F5"));
      painter->setPen(QColor("#E8E8E8"));
      painter->drawRoundedRect(rect, 5, 5);

      painter->setPen(QColor("#BFBFBF"));
      painter->setFont(QFont("Microsoft YaHei", 8));
      painter->drawText(rect, Qt::AlignCenter,
                        viewTag == "main" ? "主视角加载中..."
                                          : "侧视角加载中...");
    } else {
      // 缩放并缓存
      scaledPix = originalPix.scaled(rect.size(), Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);
      QPixmapCache::insert(cacheKey, scaledPix);
      renderImage(painter, rect, scaledPix);
    }
  } else {
    renderImage(painter, rect, scaledPix);
  }
}

void QualityCardDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {
  painter->save();
  painter->setRenderHint(QPainter::Antialiasing);
  painter->setRenderHint(QPainter::SmoothPixmapTransform);

  const int sideMargin = 15;
  const int topMargin = 20; // 给蓝色时间条留出的顶部空间
  const int bottomMargin = 10;
  QRect cardRect =
      option.rect.adjusted(sideMargin, topMargin, -sideMargin, -bottomMargin);
  // 1. 获取数据 (Role值请根据你的 Model 调整)
  QString timeText = index.data(Qt::UserRole + 1).toString(); // "2026-03-26..."
  QString titleText = index.data(Qt::UserRole + 2).toString(); // "LJ09 | 首检"
  QString mainImageUrl = index.data(Qt::UserRole + 3).toString(); // 主视角
  int channelStatus = index.data(Qt::UserRole + 4).toInt();       // 通道结果
  int qualityStatus = index.data(Qt::UserRole + 5).toInt();       // 质控结果
  QString sideImageUrl = index.data(Qt::UserRole + 6).toString(); // 侧视角

  // 2. 绘制卡片白色背景
  painter->setPen(Qt::NoPen);
  painter->setBrush(Qt::white);
  painter->drawRoundedRect(cardRect, 10, 10);

  // 3. 绘制悬浮时间条 (Blue Badge)
  QRect timeRect(cardRect.left() + 15, cardRect.top() - 12, 200, 28);
  painter->setBrush(QColor("#3BA0FF"));
  painter->drawRoundedRect(timeRect, 6, 6);
  painter->setPen(Qt::white);
  painter->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
  painter->drawText(timeRect, Qt::AlignCenter, timeText);

  // 4. 绘制标题文字
  painter->setPen(QColor("#333333"));
  painter->setFont(QFont("Microsoft YaHei", 11, QFont::DemiBold));
  painter->drawText(cardRect.left() + 15, cardRect.top() + 42, titleText);

  // 5. 绘制 X 光图片区域
  QRect totalImageArea = cardRect.adjusted(10, 60, -10, -10);
  int spacing = 8; // 图片间距
  int imgW = (totalImageArea.width() - spacing) / 2;

  QRect mainRect(totalImageArea.left(), totalImageArea.top(), imgW,
                 totalImageArea.height());
  QRect sideRect(mainRect.right() + spacing, totalImageArea.top(), imgW,
                 totalImageArea.height());

  drawAsyncImage(painter, mainRect, mainImageUrl, index, "main");
  drawAsyncImage(painter, sideRect, sideImageUrl, index, "side");

  // QPixmap pix = ImageProvider::instance().cached(imageUrl);
  // // 生成唯一的缓存键：URL + 目标宽度 + 目标高度
  // QString cacheKey = QString("thumb_%1_%2_%3")
  //                        .arg(imageUrl)
  //                        .arg(imageArea.width())
  //                        .arg(imageArea.height());
  // QPixmap scaledPix;

  // // --- QPixmapCache 查找逻辑 ---
  // if (!QPixmapCache::find(cacheKey, &scaledPix)) {
  //   // 缓存未命中：尝试从 ImageProvider 获取原始大图
  //   QPixmap originalPix = ImageProvider::instance().cached(imageUrl);

  //   if (originalPix.isNull()) {
  //     // 图片还没下载好：发起异步请求
  //     if (!m_pendingIndexes.contains(imageUrl)) {
  //       trackPendingUrl(imageUrl, index);
  //       ImageProvider::instance().request(imageUrl);
  //     }
  //     // 绘制占位 Loading 框
  //     painter->setBrush(QColor("#F5F5F5"));
  //     painter->drawRoundedRect(imageArea, 5, 5);
  //   } else {
  //     // 图片已下载，但还没生成缩略图：执行缩放
  //     scaledPix = originalPix.scaled(imageArea.size(), Qt::KeepAspectRatio,
  //                                    Qt::SmoothTransformation);

  //     // 将缩放后的结果存入系统缓存
  //     QPixmapCache::insert(cacheKey, scaledPix);

  //     // 居中绘制
  //     renderImage(painter, imageArea, scaledPix);
  //   }
  // } else {
  //   // 缓存命中：极其顺滑，直接绘制，跳过所有计算
  //   renderImage(painter, imageArea, scaledPix);
  // }

  // 6. 绘制右上角状态标签 (叠加显示)
  int tagW = 85;
  int tagH = 28;
  int tagX = cardRect.right() - tagW;
  int tagY = cardRect.top();

  // 通道标签
  drawTag(painter, tagX, tagY, tagW, tagH, getChannelText(channelStatus),
          getColor(channelStatus));
  // 质控标签 (在通道标签下方)
  drawTag(painter, tagX, tagY + tagH + 6, tagW, tagH,
          getQualityText(qualityStatus), getColor(qualityStatus));

  painter->restore();
}

void QualityCardDelegate::drawTag(QPainter *painter, int x, int y, int w, int h,
                                  const QString &text,
                                  const QColor &color) const {
  painter->save();
  painter->setPen(Qt::NoPen);
  painter->setBrush(color);
  painter->drawRoundedRect(QRect(x, y, w, h), 2, 2); // 标签圆角较小

  painter->setPen(Qt::white);
  painter->setFont(QFont("Microsoft YaHei", 9, QFont::Bold));
  painter->drawText(QRect(x, y, w, h), Qt::AlignCenter, text);
  painter->restore();
}

QSize QualityCardDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const {
  Q_UNUSED(index)
  Q_UNUSED(option)
  return QSize(400, 300);
}

void QualityCardDelegate::trackPendingUrl(const QString &url,
                                          const QModelIndex &index) const {
  if (url.isEmpty())
    return;
  QPersistentModelIndex pIndex(index);
  if (!m_pendingIndexes[url].contains(pIndex)) {
    m_pendingIndexes[url].append(pIndex);
  }
}