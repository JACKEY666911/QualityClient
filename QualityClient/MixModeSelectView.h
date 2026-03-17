#ifndef MIXMODESELECTVIEW_H
#define MIXMODESELECTVIEW_H

#include <QWidget>

namespace Ui {
class MixModeSelectView;
}

class MixModeSelectView : public QWidget
{
    Q_OBJECT

public:
    explicit MixModeSelectView(QWidget *parent = nullptr);
    ~MixModeSelectView();
signals:
    void loginOut();
    void enterQualityControl();
private slots:
    void on_loginoutButton_clicked();
    void on_switchToMainViewBtn_clicked();
    void on_loginOutBtn_clicked();

private:
    Ui::MixModeSelectView *ui;
};

#endif // MIXMODESELECTVIEW_H
