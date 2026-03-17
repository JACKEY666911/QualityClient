#include "AnnotationGraphicsView.h"

#include <QApplication>
#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QMenu>
#include <QWidgetAction>
#include <QLabel>
#include <QPushButton>
#include <QWheelEvent>
#include <QtGlobal>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QPointer>
namespace {
const int kHoverDeleteDelayMs = 1000;
const qreal kMinRectSize = 5.0;
const qreal kDeleteButtonWidth = 52.0;
const qreal kDeleteButtonHeight = 20.0;
const qreal kDeleteButtonMargin = 2.0;
const qreal kLabelSpacing = 4.0;
}

AnnotationRectItem::AnnotationRectItem(const QRectF &rect, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent)
    , m_label(new QGraphicsSimpleTextItem(this))
    , m_deleteButton(new QGraphicsRectItem(this))
    , m_deleteText(new QGraphicsSimpleTextItem(m_deleteButton))
    , m_hoverTimer(new QTimer(this))
    , m_isManual(true)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setPen(QPen(QColor("#8400FF"), 5));
    setBrush(Qt::NoBrush);

    QFont labelFont;
    labelFont.setPointSize(12);
    labelFont.setBold(true);
    m_label->setFont(labelFont);
    m_label->setBrush(QBrush(QColor("#8400FF")));
    updateLabelStyle();

    m_deleteButton->setBrush(QBrush(QColor(220, 60, 60)));
    m_deleteButton->setPen(QPen(QColor(180, 40, 40), 1));
    m_deleteButton->setVisible(false);
    m_deleteButton->setZValue(2);

    QFont deleteFont;
    deleteFont.setPointSize(9);
    deleteFont.setBold(true);
    m_deleteText->setFont(deleteFont);
    m_deleteText->setBrush(QBrush(Qt::white));
    m_deleteText->setText(QStringLiteral("DELETE"));

    m_hoverTimer->setSingleShot(true);
    connect(m_hoverTimer, &QTimer::timeout, this, &AnnotationRectItem::handleHoverTimeout);

    updateDecorations();
}

void AnnotationRectItem::setCategory(const QString &category, const QString &code)
{
    m_category = category;
    if (!code.isEmpty()) {
        m_code = code;
    } else if (m_code.isEmpty()) {
        m_code = category;
    }
    m_label->setText(m_category);
    updateLabelPosition();
}

void AnnotationRectItem::updateDecorations()
{
    updatePenStyle();
    updateLabelPosition();
    updateDeleteButtonPosition();
}

bool AnnotationRectItem::isDeleteButtonVisible() const
{
    return m_deleteButton->isVisible();
}

bool AnnotationRectItem::isOnDeleteButton(const QPointF &scenePos) const
{
    if (!m_deleteButton->isVisible()) {
        return false;
    }
    QPointF localPos = mapFromScene(scenePos);
    QPointF buttonLocal = localPos - m_deleteButton->pos();
    return m_deleteButton->rect().contains(buttonLocal);
}

void AnnotationRectItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_hoverTimer->start(kHoverDeleteDelayMs);
}

void AnnotationRectItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_hoverTimer->stop();
    showDeleteButton(false);
}

void AnnotationRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QStyleOptionGraphicsItem opt(*option);
    opt.state &= ~QStyle::State_Selected;
    opt.state &= ~QStyle::State_HasFocus;
    QGraphicsRectItem::paint(painter, &opt, widget);
}

void AnnotationRectItem::handleHoverTimeout()
{
    showDeleteButton(true);
}

void AnnotationRectItem::showDeleteButton(bool show)
{
    m_deleteButton->setVisible(show && m_isManual);
}

void AnnotationRectItem::updateDeleteButtonPosition()
{
    QRectF rect = this->rect();
    m_deleteButton->setRect(0, 0, kDeleteButtonWidth, kDeleteButtonHeight);
    m_deleteButton->setPos(rect.right() - kDeleteButtonWidth, rect.top() - kDeleteButtonHeight - kDeleteButtonMargin);
    m_deleteText->setPos(6, 2);
}

void AnnotationRectItem::updateLabelPosition()
{
    QRectF rect = this->rect();
    QRectF labelRect = m_label->boundingRect();
    qreal x = rect.left() + (rect.width() - labelRect.width()) / 2.0;
    qreal yBelow = rect.bottom() + kLabelSpacing;
    qreal yAbove = rect.top() - kLabelSpacing - labelRect.height();
    qreal y = yBelow;
    if (scene()) {
        QRectF sceneRect = scene()->sceneRect();
        if (yBelow + labelRect.height() > sceneRect.bottom() && yAbove >= sceneRect.top()) {
            y = yAbove;
        }
    }
    m_label->setPos(x, y);
}

void AnnotationRectItem::updateLabelStyle()
{
    m_label->setZValue(1);
}

void AnnotationRectItem::updatePenStyle()
{
    if (m_isManual) {
        setPen(QPen(QColor("#8400FF"), 5));
        m_label->setBrush(QBrush(QColor("#8400FF")));
    } else {
        setPen(QPen(QColor("#6F66FF"), 5));
        m_label->setBrush(QBrush(QColor("#6F66FF")));
    }
}

void AnnotationRectItem::setManual(bool manual)
{
    m_isManual = manual;
    updatePenStyle();
}

AnnotationGraphicsView::AnnotationGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , m_scene(new QGraphicsScene(this))
    , m_imageItem(new QGraphicsPixmapItem())
    , m_hasImage(false)
    , m_drawingItem(nullptr)
    , m_isDrawing(false)
    , m_isPanning(false)
    , m_active(false)
    , m_menuTarget(nullptr)
    , m_zoomFactor(1.0)
    , m_baseScale(1.0)
    , m_showManual(true)
    , m_showAi(true)
{
    setScene(m_scene);
    setMouseTracking(true);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    setFrameShape(QFrame::NoFrame);
    setBackgroundBrush(QBrush(QColor("#F2F2F2")));
    setFocusPolicy(Qt::StrongFocus);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setDragMode(QGraphicsView::NoDrag);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setActive(false);

    m_scene->addItem(m_imageItem);
    m_imageItem->setZValue(-1);
    updateSceneRect();

    QList<CategoryItem> defaults;
    CategoryItem item;

    item = CategoryItem{QStringLiteral("火种"), QStringLiteral("101"),
                        QStringLiteral(":/Images/Tinder_1.png"), QStringLiteral(":/Images/Tinder_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("超量充电宝"), QStringLiteral("102"),
                        QStringLiteral(":/Images/bigBattery_1.png"), QStringLiteral(":/Images/bigBattery_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("超量液体"), QStringLiteral("103"),
                        QStringLiteral(":/Images/bigLiquid_1.png"), QStringLiteral(":/Images/bigLiquid_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("钝器"), QStringLiteral("104"),
                        QStringLiteral(":/Images/Blunt_1.png"), QStringLiteral(":/Images/Blunt_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("刀"), QStringLiteral("105"),
                        QStringLiteral(":/Images/Sharps_1.png"), QStringLiteral(":/Images/Sharps_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("工具"), QStringLiteral("106"),
                        QStringLiteral(":/Images/Tool_1.png"), QStringLiteral(":/Images/Tool_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("易燃品"), QStringLiteral("107"),
                        QStringLiteral(":/Images/Flammable_1.png"), QStringLiteral(":/Images/Flammable_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("管制器具"), QStringLiteral("108"),
                        QStringLiteral(":/Images/Regulatory_1.png"), QStringLiteral(":/Images/Regulatory_2.png")};
    defaults.append(item);
    item = CategoryItem{QStringLiteral("其他物品"), QStringLiteral("109"),
                        QStringLiteral(":/Images/Other_1.png"), QStringLiteral(":/Images/Other_2.png")};
    defaults.append(item);

    setCategoryItems(defaults);
}

void AnnotationGraphicsView::setImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        m_imageItem->setPixmap(QPixmap());
        m_hasImage = false;
        resetTransform();
        updateSceneRect();
        return;
    }
    m_imageItem->setPixmap(pixmap);
    m_hasImage = true;
    updateSceneRect();
    updateImageScale();
    emitViewRect();
}

QSizeF AnnotationGraphicsView::imageSize() const
{
    if (!m_hasImage) {
        return QSizeF();
    }
    return QSizeF(m_imageItem->pixmap().size());
}

QPixmap AnnotationGraphicsView::image() const
{
    return m_imageItem->pixmap();
}

void AnnotationGraphicsView::setActive(bool active)
{
    m_active = active;
    viewport()->update();
}

void AnnotationGraphicsView::setCategories(const QStringList &categories)
{
    QList<CategoryItem> items;
    for (const QString &name : categories) {
        CategoryItem item;
        item.name = name;
        item.code = name;
        item.normalPath.clear();
        item.pressedPath.clear();
        items.append(item);
    }
    m_categories = items;
}

void AnnotationGraphicsView::setCategoryItems(const QList<CategoryItem> &items)
{
    m_categories = items;
}

void AnnotationGraphicsView::zoomIn()
{
    if (!m_hasImage) {
        return;
    }
    if (m_zoomFactor >= 6.0) {
        return;
    }
    m_zoomFactor *= 1.1;
    scale(1.1, 1.1);
    emitViewRect();
}

void AnnotationGraphicsView::zoomOut()
{
    if (!m_hasImage) {
        return;
    }
    if (m_zoomFactor <= 1.0) {
        return;
    }
    m_zoomFactor /= 1.1;
    scale(1.0 / 1.1, 1.0 / 1.1);
    emitViewRect();
}

void AnnotationGraphicsView::resetZoom()
{
    if (!m_hasImage) {
        m_zoomFactor = 1.0;
        return;
    }
    applyFitToView();
    emitViewRect();
}

void AnnotationGraphicsView::centerOnNormalized(qreal xRatio, qreal yRatio)
{
    if (!m_hasImage) {
        return;
    }
    QSizeF size = imageSize();
    if (size.isEmpty()) {
        return;
    }
    qreal x = qBound(0.0, xRatio, 1.0) * size.width();
    qreal y = qBound(0.0, yRatio, 1.0) * size.height();
    centerOn(QPointF(x, y));
    emitViewRect();
}

QList<AnnotationGraphicsView::AnnotationData> AnnotationGraphicsView::annotations() const
{
    QList<AnnotationData> list;
    if (!m_hasImage) {
        return list;
    }
    const QList<QGraphicsItem *> items = m_scene->items();
    for (QGraphicsItem *item : items) {
        if (item->type() != AnnotationRectItem::Type) {
            continue;
        }
        const AnnotationRectItem *rectItem = static_cast<AnnotationRectItem *>(item);
        AnnotationData data;
        data.category = rectItem->category();
        data.code = rectItem->code();
        data.isManual = rectItem->isManual();
        data.imageRect = toImageRect(rectItem->rect());
        list.append(data);
    }
    return list;
}

void AnnotationGraphicsView::addAiAnnotation(const QRectF &imageRect, const QString &category, const QString &code)
{
    if (!m_hasImage) {
        return;
    }
    QRectF sceneRect = toSceneRect(imageRect);
    AnnotationRectItem *item = new AnnotationRectItem(sceneRect);
    item->setManual(false);
    item->setCategory(category, code);
    item->setVisible(m_showAi);
    m_scene->addItem(item);
}

void AnnotationGraphicsView::setManualVisible(bool visible)
{
    m_showManual = visible;
    const QList<QGraphicsItem *> items = m_scene->items();
    for (QGraphicsItem *item : items) {
        if (item->type() != AnnotationRectItem::Type) {
            continue;
        }
        AnnotationRectItem *rectItem = static_cast<AnnotationRectItem *>(item);
        if (rectItem->isManual()) {
            rectItem->setVisible(visible);
        }
    }
}

void AnnotationGraphicsView::setAiVisible(bool visible)
{
    m_showAi = visible;
    const QList<QGraphicsItem *> items = m_scene->items();
    for (QGraphicsItem *item : items) {
        if (item->type() != AnnotationRectItem::Type) {
            continue;
        }
        AnnotationRectItem *rectItem = static_cast<AnnotationRectItem *>(item);
        if (!rectItem->isManual()) {
            rectItem->setVisible(visible);
        }
    }
}

void AnnotationGraphicsView::refreshViewRect()
{
    emitViewRect();
}

void AnnotationGraphicsView::clearAnnotations()
{
    QList<QGraphicsItem *> items = m_scene->items();
    for (QGraphicsItem *item : items) {
        if (item->type() == AnnotationRectItem::Type) {
            deleteItem(static_cast<AnnotationRectItem *>(item));
        }
    }
}

void AnnotationGraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        if (!m_active) {
            emit activated(this);
        }
        if (m_hasImage && isZoomed()) {
            m_isPanning = true;
            m_lastPanScenePos = mapToScene(event->pos());
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
    }
    if (event->button() == Qt::LeftButton) {
        if (!m_active) {
            emit activated(this);
        }
        if (!m_hasImage) {
            event->accept();
            return;
        }
        AnnotationRectItem *clickedItem = itemAtViewPos(event->pos());
        if (clickedItem && !m_isDrawing) {
            QPointF scenePos = mapToScene(event->pos());
            if (clickedItem->isDeleteButtonVisible() && clickedItem->isOnDeleteButton(scenePos)) {
                deleteItem(clickedItem);
                event->accept();
                return;
            }
            clickedItem->setSelected(true);
            showCategoryMenu(clickedItem, event->globalPos());
            event->accept();
            return;
        }
        QPoint clampedPress = event->pos();
        clampedPress.setX(qBound(0, clampedPress.x(), viewport()->width() - 1));
        clampedPress.setY(qBound(0, clampedPress.y(), viewport()->height() - 1));
        m_startScenePos = mapToScene(clampedPress);
        m_drawingItem = new AnnotationRectItem(QRectF(m_startScenePos, m_startScenePos));
        m_scene->addItem(m_drawingItem);
        m_drawingItem->setManual(true);
        m_drawingItem->setSelected(true);
        m_isDrawing = true;
        event->accept();
        return;
    }
    QGraphicsView::mousePressEvent(event);
}

void AnnotationGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPointF currentScenePos = mapToScene(event->pos());
        QPointF deltaScene = currentScenePos - m_lastPanScenePos;
        m_lastPanScenePos = currentScenePos;
        centerOn(mapToScene(viewport()->rect().center()) - deltaScene);
        emitViewRect();
        event->accept();
        return;
    }
    if (m_isDrawing && m_drawingItem) {
        QPoint clampedPos = event->pos();
        clampedPos.setX(qBound(0, clampedPos.x(), viewport()->width() - 1));
        clampedPos.setY(qBound(0, clampedPos.y(), viewport()->height() - 1));
        QRectF rect = normalizedRect(m_startScenePos, mapToScene(clampedPos));
        m_drawingItem->setRect(rect);
        m_drawingItem->updateDecorations();
        event->accept();
        return;
    }
    QGraphicsView::mouseMoveEvent(event);
}

void AnnotationGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton && m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }
    if (m_isDrawing && m_drawingItem && event->button() == Qt::LeftButton) {
        QPoint clampedPos = event->pos();
        clampedPos.setX(qBound(0, clampedPos.x(), viewport()->width() - 1));
        clampedPos.setY(qBound(0, clampedPos.y(), viewport()->height() - 1));
        QRectF rect = normalizedRect(m_startScenePos, mapToScene(clampedPos));
        if (rect.width() < kMinRectSize || rect.height() < kMinRectSize) {
            deleteItem(m_drawingItem);
        } else {
            m_drawingItem->setRect(rect);
            m_drawingItem->updateDecorations();
            QPointF menuPosScene(rect.right(), rect.center().y());
            QPoint menuPos = mapToGlobal(mapFromScene(menuPosScene));
            showCategoryMenu(m_drawingItem, menuPos);
        }
        m_drawingItem = nullptr;
        m_isDrawing = false;
        event->accept();
        return;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void AnnotationGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        if (!m_hasImage) {
            event->accept();
            return;
        }
        QPoint viewPos = mapFromGlobal(QCursor::pos());
        AnnotationRectItem *target = itemAtViewPos(viewPos);
        if (target) {
            deleteItem(target);
        }
        event->accept();
        return;
    }
    QGraphicsView::keyPressEvent(event);
}

void AnnotationGraphicsView::focusInEvent(QFocusEvent *event)
{
    emit activated(this);
    QGraphicsView::focusInEvent(event);
}

void AnnotationGraphicsView::wheelEvent(QWheelEvent *event)
{
    if (!m_hasImage) {
        event->accept();
        return;
    }
    if (event->angleDelta().y() > 0) {
        zoomIn();
    } else if (event->angleDelta().y() < 0) {
        zoomOut();
    }
    event->accept();
}

void AnnotationGraphicsView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}

void AnnotationGraphicsView::resizeEvent(QResizeEvent *event)
{
    if (!m_hasImage) {
        updateSceneRect();
    }
    if (m_hasImage) {
        updateImageScale();
    }
    emitViewRect();
    QGraphicsView::resizeEvent(event);
}

AnnotationRectItem *AnnotationGraphicsView::itemAtViewPos(const QPoint &viewPos) const
{
    QGraphicsItem *item = itemAt(viewPos);
    while (item) {
        if (item->type() == AnnotationRectItem::Type) {
            return static_cast<AnnotationRectItem *>(item);
        }
        item = item->parentItem();
    }
    return nullptr;
}

QRectF AnnotationGraphicsView::normalizedRect(const QPointF &start, const QPointF &end) const
{
    QRectF rect(start, end);
    return rect.normalized();
}

void AnnotationGraphicsView::showCategoryMenu(AnnotationRectItem *item, const QPoint &globalPos)
{
    if (!item) {
        return;
    }
    m_menuTarget = item;
    QMenu menu(this);
    QAction *chosen = nullptr;
    menu.setFixedWidth(190);
    for (const CategoryItem &cat : m_categories) {
        QWidgetAction *action = new QWidgetAction(&menu);
        QPushButton *button = new QPushButton;
        button->setFixedSize(190, 53);
        button->setFlat(true);
        QString normal = cat.normalPath;
        QString pressed = cat.pressedPath.isEmpty() ? cat.normalPath : cat.pressedPath;
        if (normal.isEmpty()) {
            button->setStyleSheet(QStringLiteral("QPushButton{border:none;background:#E8E8E8;}"));
        } else {
            QString style = QStringLiteral(
                "QPushButton{border:none;border-image:url(%1) 0 0 0 0 stretch stretch;}"
                "QPushButton:pressed{border-image:url(%2) 0 0 0 0 stretch stretch;}"
            ).arg(normal, pressed);
            button->setStyleSheet(style);
        }
        action->setDefaultWidget(button);
        action->setData(cat.code);
        action->setText(cat.name);
        menu.addAction(action);
        QPointer<QMenu> menuPtr(&menu);
        connect(button, &QPushButton::clicked, this, [menuPtr, action, &chosen]() {
            if (menuPtr) {  // 检查菜单是否还存在
                chosen = action;
                menuPtr->close();
            }
        });
    }
    menu.exec(globalPos);
    if (chosen) {
        applyCategory(m_menuTarget, chosen->text(), chosen->data().toString());
    } else if (m_menuTarget && m_menuTarget->category().isEmpty()) {
        applyCategory(m_menuTarget, QStringLiteral("其他物品"), QStringLiteral("OTHER"));
    }
    m_menuTarget = nullptr;
}

void AnnotationGraphicsView::applyCategory(AnnotationRectItem *item, const QString &category)
{
    if (!item) {
        return;
    }
    item->setCategory(category, category);
}

void AnnotationGraphicsView::applyCategory(AnnotationRectItem *item, const QString &category, const QString &code)
{
    if (!item) {
        return;
    }
    item->setCategory(category, code);
}

void AnnotationGraphicsView::deleteItem(AnnotationRectItem *item)
{
    if (!item) {
        return;
    }
    m_scene->removeItem(item);
    delete item;
}

void AnnotationGraphicsView::updateSceneRect()
{
    QRectF rect;
    if (m_hasImage) {
        rect = QRectF(QPointF(0, 0), m_imageItem->pixmap().size());
    } else {
        rect = QRectF(0, 0, width(), height());
    }
    m_scene->setSceneRect(rect);
}

void AnnotationGraphicsView::applyFitToView()
{
    if (!m_hasImage) {
        return;
    }
    resetTransform();
    fitInView(m_imageItem, Qt::KeepAspectRatio);
    m_baseScale = transform().m11();
    m_zoomFactor = 1.0;
}

void AnnotationGraphicsView::updateImageScale()
{
    if (!m_hasImage) {
        return;
    }
    QSizeF imgSize = m_imageItem->pixmap().size();
    if (imgSize.isEmpty()) {
        return;
    }
    QSizeF viewSize = viewport()->size();
    if (viewSize.isEmpty()) {
        return;
    }
    qreal sx = viewSize.width() / imgSize.width();
    qreal sy = viewSize.height() / imgSize.height();
    qreal scaleValue = qMin(sx, sy);

    resetTransform();
    scale(scaleValue, scaleValue);
    centerOn(imgSize.width() / 2.0, imgSize.height() / 2.0);

    m_baseScale = scaleValue;
    m_zoomFactor = 1.0;
}

void AnnotationGraphicsView::emitViewRect()
{
    if (!m_hasImage) {
        emit viewRectChanged(QRectF());
        return;
    }
    QRect viewRect = viewport()->rect();
    QRectF sceneRect = mapToScene(viewRect).boundingRect();
    QRectF imageRect(QPointF(0, 0), m_imageItem->pixmap().size());
    QRectF visible = sceneRect.intersected(imageRect);
    if (visible.isEmpty() || imageRect.isEmpty()) {
        emit viewRectChanged(QRectF());
        return;
    }
    QRectF normalized(
        visible.left() / imageRect.width(),
        visible.top() / imageRect.height(),
        visible.width() / imageRect.width(),
        visible.height() / imageRect.height()
    );
    emit viewRectChanged(normalized);
}

QRectF AnnotationGraphicsView::toImageRect(const QRectF &sceneRect) const
{
    if (!m_hasImage) {
        return QRectF();
    }
    return sceneRect;
}

QRectF AnnotationGraphicsView::toSceneRect(const QRectF &imageRect) const
{
    if (!m_hasImage) {
        return QRectF();
    }
    return imageRect;
}
