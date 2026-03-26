#include "XrayImageButton.h"

#include <QImage>
#include <QPainter>

XrayImageButton::XrayImageButton(int typeId, const QString &label, QWidget *parent)
    : QPushButton(parent)
    , m_typeId(typeId)
    , m_state(State::Unavailable)
{
    setFlat(true);
    setCheckable(false);
    setFocusPolicy(Qt::NoFocus);
    setFixedSize(47, 47);
    setText(QString());
    setToolTip(label);
    setAccessibleName(label);
    setStyleSheet(QStringLiteral("QPushButton{border:none;background:transparent;}"));
}

int XrayImageButton::typeId() const
{
    return m_typeId;
}

void XrayImageButton::setImages(const QPixmap &unavailable, const QPixmap &available, const QPixmap &selected)
{
    m_unavailablePixmap = unavailable;
    m_availablePixmap = available;
    m_selectedPixmap = selected;

    if (m_unavailablePixmap.isNull() && !m_availablePixmap.isNull()) {
        m_unavailablePixmap = makeGray(m_availablePixmap);
    }
    if (m_selectedPixmap.isNull() && !m_availablePixmap.isNull()) {
        m_selectedPixmap = m_availablePixmap;
    }
    applyState();
}

void XrayImageButton::setImages(const QString &unavailablePath, const QString &availablePath, const QString &selectedPath)
{
    QPixmap unavailable(unavailablePath);
    QPixmap available(availablePath);
    QPixmap selected(selectedPath);
    setImages(unavailable, available, selected);
}

void XrayImageButton::setAvailable(bool available)
{
    if (available) {
        if (m_state == State::Unavailable) {
            m_state = State::Available;
        }
    } else {
        m_state = State::Unavailable;
    }
    applyState();
}

bool XrayImageButton::isAvailable() const
{
    return m_state != State::Unavailable;
}

void XrayImageButton::setSelected(bool selected)
{
    if (selected) {
        m_state = State::Selected;
    } else if (m_state == State::Selected) {
        m_state = State::Available;
    }
    applyState();
}

bool XrayImageButton::isSelected() const
{
    return m_state == State::Selected;
}

XrayImageButton::State XrayImageButton::state() const
{
    return m_state;
}

void XrayImageButton::setState(State state)
{
    m_state = state;
    applyState();
}

void XrayImageButton::applyState()
{
    QPixmap pixmap;
    switch (m_state) {
    case State::Unavailable:
        pixmap = m_unavailablePixmap;
        break;
    case State::Available:
        pixmap = m_availablePixmap;
        break;
    case State::Selected:
        pixmap = m_selectedPixmap;
        break;
    }
    if (!pixmap.isNull()) {
        setIcon(QIcon(pixmap));
        setIconSize(QSize(47, 47));
    } else {
        setIcon(QIcon());
    }
}

QPixmap XrayImageButton::makeGray(const QPixmap &source)
{
    if (source.isNull()) {
        return {};
    }
    QPixmap result = source;

    QPainter painter(&result);
    painter.fillRect(result.rect(), QColor(229, 229, 229, 160)); // 白色，约63%透明度
    painter.end();

    return result;
}
