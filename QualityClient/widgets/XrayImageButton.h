#ifndef XRAYIMAGEBUTTON_H
#define XRAYIMAGEBUTTON_H

#include <QPushButton>
#include <QPixmap>

class XrayImageButton : public QPushButton
{
    Q_OBJECT
public:
    enum class State {
        Unavailable,
        Available,
        Selected
    };

    explicit XrayImageButton(int typeId, const QString &label, QWidget *parent = nullptr);

    int typeId() const;

    void setImages(const QPixmap &unavailable, const QPixmap &available, const QPixmap &selected);
    void setImages(const QString &unavailablePath, const QString &availablePath, const QString &selectedPath);

    void setAvailable(bool available);
    bool isAvailable() const;

    void setSelected(bool selected);
    bool isSelected() const;

    State state() const;
    void setState(State state);

private:
    void applyState();
    static QPixmap makeGray(const QPixmap &source);

    int m_typeId;
    State m_state;
    QPixmap m_unavailablePixmap;
    QPixmap m_availablePixmap;
    QPixmap m_selectedPixmap;
};

#endif // XRAYIMAGEBUTTON_H
