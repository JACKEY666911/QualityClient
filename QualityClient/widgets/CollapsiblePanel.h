#ifndef COLLAPSIBLEPANEL_H
#define COLLAPSIBLEPANEL_H

#include <QWidget>

class QPushButton;

class CollapsiblePanel : public QWidget
{
    Q_OBJECT
public:
    explicit CollapsiblePanel(QWidget *parent = nullptr);

    void setContentWidget(QWidget *content);
    QWidget *contentWidget() const;

    bool isCollapsed() const;
    void setCollapsed(bool collapsed);

signals:
    void collapsedChanged(bool collapsed);

protected:
    QSize sizeHint() const override;

private:
    QSize contentSize() const;
    void updateGeometryForState();

    QWidget *m_content;
    QPushButton *m_toggleButton;
    bool m_collapsed;
    int m_toggleWidth;
};

#endif // COLLAPSIBLEPANEL_H
