#include "CollapsiblePanel.h"

#include <QHBoxLayout>
#include <QPushButton>

CollapsiblePanel::CollapsiblePanel(QWidget *parent)
    : QWidget(parent),
      m_content(nullptr),
      m_toggleButton(nullptr),
      m_collapsed(false),
      m_toggleWidth(25)
{
    setObjectName(QStringLiteral("collapsiblePanel"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_toggleButton = new QPushButton(QStringLiteral("收\n起"), this);
    m_toggleButton->setFixedWidth(m_toggleWidth);
    m_toggleButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    m_toggleButton->setObjectName(QStringLiteral("collapsibleToggle"));
    connect(m_toggleButton, &QPushButton::clicked, this, [this]() {
        setCollapsed(!m_collapsed);
    });

    layout->addWidget(m_toggleButton);
}

void CollapsiblePanel::setContentWidget(QWidget *content)
{
    if (m_content == content) {
        return;
    }
    if (m_content) {
        m_content->setParent(nullptr);
    }
    m_content = content;
    if (m_content) {
        m_content->setParent(this);
        layout()->removeWidget(m_toggleButton);
        layout()->addWidget(m_content);
        layout()->addWidget(m_toggleButton);
    }
    updateGeometryForState();
}

QWidget *CollapsiblePanel::contentWidget() const
{
    return m_content;
}

bool CollapsiblePanel::isCollapsed() const
{
    return m_collapsed;
}

void CollapsiblePanel::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed) {
        return;
    }
    m_collapsed = collapsed;
    if (m_content) {
        m_content->setVisible(!m_collapsed);
    }
    m_toggleButton->setText(m_collapsed ? QStringLiteral("展\n开") : QStringLiteral("收\n起"));
    updateGeometryForState();
    emit collapsedChanged(m_collapsed);
}

QSize CollapsiblePanel::sizeHint() const
{
    QSize content = contentSize();
    int h = content.height();
    if (m_collapsed) {
        return QSize(m_toggleWidth, h > 0 ? h : QWidget::sizeHint().height());
    }
    int w = m_toggleWidth;
    if (m_content) {
        w += content.width();
        h = content.height();
    }
    h = qMax(h, QWidget::sizeHint().height());
    return QSize(w, h);
}

void CollapsiblePanel::updateGeometryForState()
{
    if (m_content) {
        int h = contentSize().height();
        if (h > 0) {
            setFixedHeight(h);
            m_toggleButton->setFixedHeight(h);
        }
    }
    if (m_collapsed) {
        setFixedWidth(m_toggleWidth);
        setMaximumWidth(m_toggleWidth);
    } else if (m_content) {
        int w = contentSize().width() + m_toggleWidth;
        setFixedWidth(w);
        setMaximumWidth(w);
    }
    updateGeometry();
}

QSize CollapsiblePanel::contentSize() const
{
    if (!m_content) {
        return QSize();
    }
    QSize s = m_content->sizeHint();
    QSize minHint = m_content->minimumSizeHint();
    QSize minSize = m_content->minimumSize();
    QSize maxSize = m_content->maximumSize();

    QSize best = s;
    if (minHint.isValid()) {
        best.setWidth(qMax(best.width(), minHint.width()));
        best.setHeight(qMax(best.height(), minHint.height()));
    }
    if (minSize.isValid()) {
        best.setWidth(qMax(best.width(), minSize.width()));
        best.setHeight(qMax(best.height(), minSize.height()));
    }
    if (maxSize.isValid() && maxSize.width() == minSize.width() && maxSize.height() == minSize.height()
        && maxSize.width() > 0 && maxSize.height() > 0) {
        best = maxSize;
    }
    if (best.width() <= 0 || best.height() <= 0) {
        QSize current = m_content->size();
        if (current.isValid() && !current.isEmpty()) {
            best = current;
        }
    }
    return best;
}
