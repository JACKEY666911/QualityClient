#include "qualitycarddelegate.h"
#include "Models/QueryInfo.h"
#include "Models/GlobalEnums.h"
#include "services/imageprovider.h"

#include <QAbstractItemView>
#include <QPainterPath>
#include <QUrl>

QualityCardDelegate::QualityCardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    connect(&ImageProvider::instance(), &ImageProvider::imageUpdated, this, [this](const QString &url, bool success) {
        Q_UNUSED(success);
        auto it = m_pendingIndexes.find(url);
        if (it == m_pendingIndexes.end()) {
            return;
        }

        if (QAbstractItemView *view = qobject_cast<QAbstractItemView*>(this->parent())) {
            for (const QPersistentModelIndex &persistentIndex : it.value()) {
                if (!persistentIndex.isValid()) {
                    continue;
                }
                const QRect rect = view->visualRect(persistentIndex);
                if (rect.isValid()) {
                    view->viewport()->update(rect);
                }
            }
        }
        m_pendingIndexes.erase(it);
    });
}

QSize QualityCardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(400, 260);
}

void QualityCardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::SmoothPixmapTransform);

    QueryInfo* data = index.data(Qt::UserRole + 1).value<QueryInfo*>();
    if (!data) return;

    bool isSelected = option.state & QStyle::State_Selected;
    bool isMouseOver = option.state & QStyle::State_MouseOver;
    QRect rect = option.rect.adjusted(10, 10, -10, -10);

    // --------------------------
    // 选中阴影（最底层，不覆盖边框）
    // --------------------------
    if (isSelected) {
        painter->save();
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(45, 170, 255, 35));
        painter->drawRoundedRect(rect.adjusted(-4, -4, 4, 4), 12, 12);
        painter->restore();
    }

    // --------------------------
    // 卡片背景 + 边框
    // --------------------------
    painter->save();
    QPainterPath cardPath;
    cardPath.addRoundedRect(rect, 10, 10);

    QColor bg = Qt::white;
    QColor border = QStringLiteral("#d7d7d7");
    int borderWidth = 1;

    if (isSelected) {
        border = QStringLiteral("#2DAAFF");
        borderWidth = 2;
    }
    if (isMouseOver) {
        border =  QStringLiteral("#7FBFF5");
    }

    painter->fillPath(cardPath, bg);
    painter->setPen(QPen(border, borderWidth));
    painter->drawPath(cardPath);
    painter->restore();

    // --------------------------
    // 右上角：通道标签
    // --------------------------
    int tagW = 100;
    int tagH = 28;
    int tagX = rect.right() - tagW - 12;
    int tagY = rect.top() + 12;

    drawTag(painter, tagX, tagY, tagW, tagH,
            getChannelText(data->imageResult),
            getColor(data->imageResult));

    // --------------------------
    // 右上角：质控标签
    // --------------------------
    drawTag(painter, tagX, tagY + tagH + 4, tagW, tagH,
            getQualityText(data->qualityResult),
            getColor(data->qualityResult));

    // --------------------------
    // 左上角时间标签
    // --------------------------
    painter->save();
    QRect timeRect(rect.left() + 18, rect.top() + 12, 160, 28);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(tr("#2DAAFF")));
    painter->drawRoundedRect(timeRect, 6, 6);
    painter->setPen(Qt::white);
    painter->setFont(QFont("Microsoft YaHei", 14, QFont::Medium));
    painter->drawText(timeRect, Qt::AlignCenter, data->qualityTime);
    painter->restore();

    // --------------------------
    // 文字：通道 | 首检/复检
    // --------------------------
    painter->save();
    painter->setPen("#222222");
    painter->setFont(QFont("Microsoft YaHei", 14));
    QString text = QString("%1 | %2").arg(data->channelNoValue, data->getRecheckResultText());
    painter->drawText(rect.adjusted(15, 55, -15, 0), Qt::AlignLeft, text);
    painter->restore();

    // --------------------------
    // 左右两张图片（等比例、不变形、居中）
    // --------------------------
    painter->save();
    QRect imgArea = rect.adjusted(10, 95, -10, -10);
    int half = imgArea.width() / 2;

    drawCenteredPixmap(painter, imgArea.adjusted(0, 0, -half, 0), index, data->xrayImage.fullXrayImageUrl());
    drawCenteredPixmap(painter, imgArea.adjusted(half, 0, 0, 0), index, data->xrayImage.assistXrayImageUrl());
    painter->restore();
}

void QualityCardDelegate::drawCenteredPixmap(QPainter *painter, const QRect &rect, const QModelIndex &index, const QString &imagePathOrUrl) const
{
    const QPixmap pix = resolvePixmap(imagePathOrUrl, index);
    if (pix.isNull()) return;

    QPixmap scaled = pix.scaled(rect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPoint pos = rect.center() - scaled.rect().center();
    painter->drawPixmap(pos.x(), pos.y(), scaled);
}

QPixmap QualityCardDelegate::resolvePixmap(const QString &imagePathOrUrl, const QModelIndex &index) const
{
    if (imagePathOrUrl.isEmpty()) {
        return QPixmap();
    }

    const QPixmap cachedPixmap = ImageProvider::instance().cached(imagePathOrUrl);
    if (!cachedPixmap.isNull()) {
        return cachedPixmap;
    }
    const QUrl url(imagePathOrUrl);
    if (url.isValid() && (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https"))) {
        trackPendingUrl(imagePathOrUrl, index);
        ImageProvider::instance().request(imagePathOrUrl);
        return QPixmap();
    }

    return QPixmap(imagePathOrUrl);
}

void QualityCardDelegate::trackPendingUrl(const QString &url, const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    QList<QPersistentModelIndex> &indexes = m_pendingIndexes[url];
    for (const QPersistentModelIndex &existing : indexes) {
        if (existing == index) {
            return;
        }
    }
    indexes.append(QPersistentModelIndex(index));
}

void QualityCardDelegate::drawTag(QPainter *painter, int x, int y, int w, int h,
                                  const QString &text, const QColor &color) const
{
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRoundedRect(x, y, w, h, 6, 6);

    painter->setPen(Qt::white);
    painter->setFont(QFont("Microsoft YaHei", 13, QFont::Medium));
    painter->drawText(QRect(x, y, w, h), Qt::AlignCenter, text);
    painter->restore();
}

QColor QualityCardDelegate::getColor(int res) const
{
    if (res == QualityImageResult::StartCheck)
        return QColor(255, 0, 0);

    if (res == QualityImageResult::EndCheck)
        return QColor(118, 200, 43);

    return QColor(153, 153, 153);
}

QString QualityCardDelegate::getChannelText(int res) const
{
    switch (res) {
    case QualityImageResult::StartCheck: return "通道开检";
    case QualityImageResult::EndCheck:   return "通道放行";
    default: return "通道未知";
    }
}

QString QualityCardDelegate::getQualityText(int res) const
{
    switch (res) {
    case QualityImageResult::StartCheck: return "质控开检";
    case QualityImageResult::EndCheck:   return "质控放行";
    default: return "质控未知";
    }
}
