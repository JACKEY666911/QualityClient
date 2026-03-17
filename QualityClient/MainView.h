#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "LoginView.h"
#include "MixModeSelectView.h"
#include "QualityControlView.h"

#include <QWidget>
#include <QStackedWidget>
namespace Ui {
class MainView;
}

class MainView : public QWidget
{
    Q_OBJECT

public:
    explicit MainView(QWidget *parent = nullptr);
    ~MainView();

private slots:
    void handleLoginSuccess(const QString &userName);
    void handleLoginOut();
    void handleEnterQualityControl();
    void handleSwitchModeRequest();

private:
    Ui::MainView *ui;
    QStackedWidget *m_mainStack;
    LoginView *m_loginView;
    MixModeSelectView * m_mixModeSelectView;
    QualityControlView *m_qualityControlView;
};

#endif // MAINVIEW_H
