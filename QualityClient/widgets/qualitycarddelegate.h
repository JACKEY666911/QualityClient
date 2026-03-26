#ifndef QUALITYCARDDELEGATE_H
#define QUALITYCARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHash>
#include <QList>
#include <QPersistentModelIndex>

class QPixmap;

class QualityCardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit QualityCardDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void drawCenteredPixmap(QPainter *painter, const QRect &rect, const QModelIndex &index, const QString &imagePathOrUrl) const;
    QPixmap resolvePixmap(const QString &imagePathOrUrl, const QModelIndex &index) const;
    void trackPendingUrl(const QString &url, const QModelIndex &index) const;
    void drawTag(QPainter *painter, int x, int y, int w, int h, const QString &text, const QColor &color) const;

    QColor getColor(int res) const;
    QString getChannelText(int res) const;
    QString getQualityText(int res) const;

    mutable QHash<QString, QList<QPersistentModelIndex>> m_pendingIndexes;
};

#endif
