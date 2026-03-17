#ifndef ANNOTATIONGRAPHICSVIEW_H
#define ANNOTATIONGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsPixmapItem>

class QPixmap;
class QResizeEvent;
#include <QTimer>

class AnnotationRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 101 };

    explicit AnnotationRectItem(const QRectF &rect, QGraphicsItem *parent = nullptr);

    int type() const override { return Type; }

    void setCategory(const QString &category, const QString &code = QString());
    QString category() const { return m_category; }
    QString code() const { return m_code; }

    void updateDecorations();
    bool isDeleteButtonVisible() const;
    bool isOnDeleteButton(const QPointF &scenePos) const;
    void setManual(bool manual);
    bool isManual() const { return m_isManual; }

signals:
    void requestDelete(AnnotationRectItem *item);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private slots:
    void handleHoverTimeout();

private:
    void showDeleteButton(bool show);
    void updateDeleteButtonPosition();
    void updateLabelPosition();
    void updateLabelStyle();
    void updatePenStyle();

    QString m_category;
    QString m_code;
    QGraphicsSimpleTextItem *m_label;
    QGraphicsRectItem *m_deleteButton;
    QGraphicsSimpleTextItem *m_deleteText;
    QTimer *m_hoverTimer;
    bool m_isManual;
};

class AnnotationGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    struct CategoryItem {
        QString name;
        QString code;
        QString normalPath;
        QString pressedPath;
    };

    struct AnnotationData {
        QString code;
        QString category;
        QRectF imageRect;
        bool isManual;
    };

    explicit AnnotationGraphicsView(QWidget *parent = nullptr);

    void setImage(const QPixmap &pixmap);
    bool hasImage() const { return m_hasImage; }
    QSizeF imageSize() const;
    QPixmap image() const;

    void setActive(bool active);
    bool isActive() const { return m_active; }

    void setCategories(const QStringList &categories);
    void setCategoryItems(const QList<CategoryItem> &items);
    void zoomIn();
    void zoomOut();
    void resetZoom();
    bool isZoomed() const { return m_zoomFactor > 1.01; }
    void centerOnNormalized(qreal xRatio, qreal yRatio);
    QList<AnnotationData> annotations() const;
    void addAiAnnotation(const QRectF &imageRect, const QString &category, const QString &code);
    void setManualVisible(bool visible);
    void setAiVisible(bool visible);
    void refreshViewRect();
    void clearAnnotations();

signals:
    void activated(AnnotationGraphicsView *view);
    void viewRectChanged(const QRectF &normalizedRect);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    AnnotationRectItem *itemAtViewPos(const QPoint &viewPos) const;
    QRectF normalizedRect(const QPointF &start, const QPointF &end) const;
    void showCategoryMenu(AnnotationRectItem *item, const QPoint &globalPos);
    void applyCategory(AnnotationRectItem *item, const QString &category);
    void applyCategory(AnnotationRectItem *item, const QString &category, const QString &code);
    void deleteItem(AnnotationRectItem *item);
    void updateSceneRect();
    QRectF toImageRect(const QRectF &sceneRect) const;
    QRectF toSceneRect(const QRectF &imageRect) const;
    void applyFitToView();
    void updateImageScale();
    void emitViewRect();

    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_imageItem;
    bool m_hasImage;
    AnnotationRectItem *m_drawingItem;
    QPointF m_startScenePos;
    bool m_isDrawing;
    bool m_isPanning;
    QPointF m_lastPanScenePos;
    bool m_active;
    AnnotationRectItem *m_menuTarget;
    QList<CategoryItem> m_categories;
    double m_zoomFactor;
    double m_baseScale;
    bool m_showManual;
    bool m_showAi;
};

#endif // ANNOTATIONGRAPHICSVIEW_H
