#include "LayerViewWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QPainterPath>
#include <QFrame>
#include <QMouseEvent>

#include "Models/XrayImage.h"

namespace {

class TriangleButton : public QPushButton
{
public:
    enum Direction { Left, Right };

    explicit TriangleButton(Direction dir, QWidget *parent = nullptr)
        : QPushButton(parent), m_dir(dir)
    {
        setFixedSize(20, 125);
        setCursor(Qt::PointingHandCursor);
        setFlat(true);
        setFocusPolicy(Qt::NoFocus);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QColor fill = Qt::black;
        if (!isEnabled()) {
            fill = QColor(QStringLiteral("#cccccc"));
        } else if (isDown()) {
            fill = QColor(QStringLiteral("#888888"));
        } else if (underMouse()) {
            fill = QColor(QStringLiteral("#444444"));
        }

        QPainterPath path;
        if (m_dir == Right) {
            path.moveTo(0, 0);
            path.lineTo(width() - 5, height() / 2);
            path.lineTo(0, height());
        } else {
            path.moveTo(width(), 0);
            path.lineTo(5, height() / 2);
            path.lineTo(width(), height());
        }
        path.closeSubpath();

        p.setBrush(fill);
        p.setPen(Qt::NoPen);
        p.drawPath(path);
    }

private:
    Direction m_dir;
};

class LayerCardWidget : public QFrame
{
    Q_OBJECT
public:
    explicit LayerCardWidget(const QString &name, int index, QWidget *parent = nullptr)
        : QFrame(parent), m_index(index)
    {
        setFixedSize(150, 110);
        setObjectName(QStringLiteral("layerCard"));
        setStyleSheet(QStringLiteral(
            "QFrame#layerCard{background:white;border:2px solid transparent;border-radius:5px;}"
        ));

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setContentsMargins(4, 4, 4, 4);
        layout->setSpacing(2);

        m_image = new QLabel(this);
        m_image->setAlignment(Qt::AlignCenter);
        m_image->setText(QStringLiteral("图层"));
        m_image->setStyleSheet(QStringLiteral("color:#999999;"));

        m_title = new QLabel(name, this);
        m_title->setAlignment(Qt::AlignCenter);
        m_title->setStyleSheet(QStringLiteral("color:#000000;font-weight:bold;font-size:11px;"));

        layout->addWidget(m_image, 1);
        layout->addWidget(m_title, 0);
    }

    void setImage(const QPixmap &pixmap)
    {
        if (pixmap.isNull()) {
            m_image->setText(QStringLiteral("图层"));
            m_image->setPixmap(QPixmap());
            return;
        }
        m_image->setPixmap(pixmap.scaled(120, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    void setTitle(const QString &name)
    {
        m_title->setText(name);
    }

    void setIndex(int index)
    {
        m_index = index;
    }

    void setSelected(bool selected)
    {
        if (selected) {
            setStyleSheet(QStringLiteral(
                "QFrame#layerCard{background:#FFF5F5F5;border:2px solid red;border-radius:5px;}"
            ));
        } else {
            setStyleSheet(QStringLiteral(
                "QFrame#layerCard{background:white;border:2px solid transparent;border-radius:5px;}"
            ));
        }
    }

signals:
    void clicked(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            emit clicked(m_index);
        }
        QFrame::mousePressEvent(event);
    }

private:
    int m_index;
    QLabel *m_image;
    QLabel *m_title;
};

} // namespace

LayerViewWidget::LayerViewWidget(QWidget *parent)
    : QWidget(parent),
      m_topImageLabel(nullptr),
      m_durationLabel(nullptr),
      m_cardsContainer(nullptr),
      m_cardsLayout(nullptr),
      m_prevButton(nullptr),
      m_nextButton(nullptr),
      m_pageSize(3),
      m_currentStartIndex(0),
      m_selectedIndex(-1)
{
    setFixedSize(750, 190);
    setStyleSheet(QStringLiteral("background:#d7d7d7;"));

    QHBoxLayout *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    QFrame *leftPanel = new QFrame(this);
    leftPanel->setFixedWidth(175);
    leftPanel->setStyleSheet(QStringLiteral("background:#d7d7d7;"));
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(6, 6, 6, 6);
    leftLayout->setSpacing(4);

    QLabel *leftTitle = new QLabel(QStringLiteral("行李外观"), leftPanel);
    leftTitle->setAlignment(Qt::AlignCenter);
    leftTitle->setFixedHeight(30);
    leftTitle->setStyleSheet(QStringLiteral("font-size:16px;font-weight:bold;color:#000000;"));

    m_topImageLabel = new QLabel(leftPanel);
    m_topImageLabel->setFixedHeight(160);
    m_topImageLabel->setAlignment(Qt::AlignCenter);
    m_topImageLabel->setText(QStringLiteral("暂无图像"));
    m_topImageLabel->setStyleSheet(QStringLiteral("background:#f5f5f5;color:#999999;"));

    leftLayout->addWidget(leftTitle);
    leftLayout->addWidget(m_topImageLabel);

    QFrame *separator = new QFrame(this);
    separator->setFixedWidth(2);
    separator->setStyleSheet(QStringLiteral("background:#c4c4c4;"));

    QFrame *rightPanel = new QFrame(this);
    rightPanel->setStyleSheet(QStringLiteral("background:#d7d7d7;"));
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(6, 6, 6, 6);
    rightLayout->setSpacing(4);

    QWidget *header = new QWidget(rightPanel);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);

    m_durationLabel = new QLabel(QStringLiteral("10秒"), header);
    m_durationLabel->setFixedWidth(60);
    m_durationLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_durationLabel->setStyleSheet(QStringLiteral("font-size:16px;font-weight:bold;color:#19a352;"));

    QLabel *rightTitle = new QLabel(QStringLiteral("通道现场图层"), header);
    rightTitle->setAlignment(Qt::AlignCenter);
    rightTitle->setStyleSheet(QStringLiteral("font-size:16px;font-weight:bold;color:#000000;"));

    headerLayout->addWidget(m_durationLabel);
    headerLayout->addWidget(rightTitle, 1);

    QFrame *headerLine = new QFrame(rightPanel);
    headerLine->setFixedHeight(1);
    headerLine->setStyleSheet(QStringLiteral("background:#c4c4c4;"));

    QWidget *content = new QWidget(rightPanel);
    QHBoxLayout *contentLayout = new QHBoxLayout(content);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(6);

    m_prevButton = new TriangleButton(TriangleButton::Left, content);
    m_nextButton = new TriangleButton(TriangleButton::Right, content);
    connect(m_prevButton, &QPushButton::clicked, this, [this]() {
        prevPage();
        emit prevClicked();
    });
    connect(m_nextButton, &QPushButton::clicked, this, [this]() {
        nextPage();
        emit nextClicked();
    });

    m_cardsContainer = new QWidget(content);
    m_cardsLayout = new QHBoxLayout(m_cardsContainer);
    m_cardsLayout->setContentsMargins(0, 0, 0, 0);
    m_cardsLayout->setSpacing(10);

    contentLayout->addWidget(m_prevButton);
    contentLayout->addWidget(m_cardsContainer, 1);
    contentLayout->addWidget(m_nextButton);

    QWidget *bottomSpacer = new QWidget(rightPanel);
    bottomSpacer->setFixedHeight(20);

    rightLayout->addWidget(header);
    rightLayout->addWidget(headerLine);
    rightLayout->addWidget(content, 1);
    rightLayout->addWidget(bottomSpacer);

    root->addWidget(leftPanel);
    root->addWidget(separator);
    root->addWidget(rightPanel, 1);

    setLayerItems(QList<LayerItemData>());
}

void LayerViewWidget::setTopImage(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        m_topImageLabel->setText(QStringLiteral("暂无图像"));
        m_topImageLabel->setPixmap(QPixmap());
        return;
    }
    m_topImageLabel->setPixmap(pixmap.scaled(m_topImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void LayerViewWidget::setDurationText(const QString &text)
{
    m_durationLabel->setText(text);
}

void LayerViewWidget::setLayerItems(const QList<LayerItemData> &items)
{
    m_layerItems = items;
    m_itemRefs.clear();
    m_currentStartIndex = 0;
    m_selectedIndex = -1;
    for (int i = 0; i < m_layerItems.size(); ++i) {
        if (m_layerItems.at(i).selected) {
            m_selectedIndex = i;
            break;
        }
    }
    if (m_selectedIndex < 0 && !m_layerItems.isEmpty()) {
        m_selectedIndex = 0;
        m_layerItems[0].selected = true;
    }
    rebuildLayerCards();
}

void LayerViewWidget::setXrayItems(const QList<XrayImage> &items)
{
    m_layerItems.clear();
    m_itemRefs = items;
    m_layerItems.reserve(items.size());
    m_currentStartIndex = 0;
    m_selectedIndex = -1;

    for (int i = 0; i < items.size(); ++i) {
        const XrayImage &img = items.at(i);
        LayerItemData data;
        data.name = img.name();
        data.selected = img.isSelected();
        if (!img.mainXrayImageUrl().isEmpty()) {
            data.image = QPixmap(img.mainXrayImageUrl());
        }
        m_layerItems.append(data);
        if (data.selected && m_selectedIndex < 0) {
            m_selectedIndex = i;
        }
    }
    if (m_selectedIndex < 0 && !m_layerItems.isEmpty()) {
        m_selectedIndex = 0;
        m_layerItems[0].selected = true;
    }
    rebuildLayerCards();
}

void LayerViewWidget::setPageSize(int size)
{
    if (size <= 0 || m_pageSize == size) {
        return;
    }
    m_pageSize = size;
    m_currentStartIndex = 0;
    rebuildLayerCards();
}

int LayerViewWidget::pageSize() const
{
    return m_pageSize;
}

int LayerViewWidget::currentStartIndex() const
{
    return m_currentStartIndex;
}

int LayerViewWidget::selectedIndex() const
{
    return m_selectedIndex;
}

void LayerViewWidget::setSelectedIndex(int index)
{
    selectIndexInternal(index);
}

void LayerViewWidget::rebuildLayerCards()
{
    ensureCardPool(m_pageSize);
    updateVisibleCards();
    updateButtons();
}

void LayerViewWidget::updateButtons()
{
    bool canPrev = m_currentStartIndex > 0;
    bool canNext = m_currentStartIndex + m_pageSize < m_layerItems.size();
    if (m_prevButton) {
        m_prevButton->setEnabled(canPrev);
    }
    if (m_nextButton) {
        m_nextButton->setEnabled(canNext);
    }
}

void LayerViewWidget::prevPage()
{
    if (m_currentStartIndex <= 0) {
        return;
    }
    m_currentStartIndex = qMax(m_currentStartIndex - m_pageSize, 0);
    updateVisibleCards();
    emit pageChanged(m_currentStartIndex);
}

void LayerViewWidget::nextPage()
{
    if (m_currentStartIndex + m_pageSize >= m_layerItems.size()) {
        return;
    }
    m_currentStartIndex = qMin(m_currentStartIndex + m_pageSize, qMax(0, m_layerItems.size() - 1));
    updateVisibleCards();
    emit pageChanged(m_currentStartIndex);
}

void LayerViewWidget::selectIndexInternal(int index)
{
    if (index < 0 || index >= m_layerItems.size()) {
        return;
    }
    if (m_selectedIndex == index) {
        return;
    }
    for (int i = 0; i < m_layerItems.size(); ++i) {
        m_layerItems[i].selected = (i == index);
    }
    if (!m_itemRefs.isEmpty()) {
        for (int i = 0; i < m_itemRefs.size(); ++i) {
            m_itemRefs[i].setSelected(i == index);
        }
    }
    m_selectedIndex = index;
    updateVisibleSelection();
    emit selectionChanged(index);
    if (index >= 0 && index < m_itemRefs.size()) {
        emit xrayItemSelected(m_itemRefs.at(index));
    }
}

void LayerViewWidget::updateVisibleSelection()
{
    int endIndex = qMin(m_currentStartIndex + m_pageSize, m_layerItems.size());
    int visibleCount = endIndex - m_currentStartIndex;
    for (int i = 0; i < visibleCount && i < m_visibleCards.size(); ++i) {
        auto card = qobject_cast<LayerCardWidget *>(m_visibleCards.at(i));
        if (!card) {
            continue;
        }
        int realIndex = m_currentStartIndex + i;
        card->setSelected(realIndex == m_selectedIndex);
    }
}

void LayerViewWidget::ensureCardPool(int count)
{
    if (count < 0) {
        count = 0;
    }
    if (m_visibleCards.size() == count) {
        return;
    }

    QLayoutItem *child;
    while ((child = m_cardsLayout->takeAt(0)) != nullptr) {
        if (QWidget *w = child->widget()) {
            w->deleteLater();
        }
        delete child;
    }
    m_visibleCards.clear();

    for (int i = 0; i < count; ++i) {
        LayerCardWidget *card = new LayerCardWidget(QStringLiteral("--"), i, m_cardsContainer);
        connect(card, &LayerCardWidget::clicked, this, [this](int index) {
            selectIndexInternal(index);
            emit layerClicked(index);
        });
        m_cardsLayout->addWidget(card);
        m_visibleCards.append(card);
    }
    m_cardsLayout->addStretch(1);
}

void LayerViewWidget::updateVisibleCards()
{
    int endIndex = qMin(m_currentStartIndex + m_pageSize, m_layerItems.size());
    int visibleCount = endIndex - m_currentStartIndex;

    for (int i = 0; i < m_visibleCards.size(); ++i) {
        auto card = qobject_cast<LayerCardWidget *>(m_visibleCards.at(i));
        if (!card) {
            continue;
        }
        int realIndex = m_currentStartIndex + i;
        if (i < visibleCount) {
            const LayerItemData &item = m_layerItems.at(realIndex);
            card->setTitle(item.name);
            card->setImage(item.image);
            card->setSelected(realIndex == m_selectedIndex);
            card->setIndex(realIndex);
            card->setVisible(true);
            card->setToolTip(item.name);
        } else {
            card->setVisible(false);
        }
    }
    updateButtons();
}

#include "LayerViewWidget.moc"
