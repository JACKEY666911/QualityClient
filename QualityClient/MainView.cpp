#include "MainView.h"
#include "ui_MainView.h"

#include <QVBoxLayout>

MainView::MainView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainView)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowState(Qt::WindowFullScreen);
    m_mainStack = new QStackedWidget(this);
    m_loginView = new LoginView(this);
    m_mixModeSelectView = new MixModeSelectView(this);
    m_mainStack->addWidget(m_loginView);
    m_mainStack->addWidget(m_mixModeSelectView);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_mainStack);
    setLayout(layout);
    connect(m_loginView, &LoginView::loginSuccess, this, &MainView::handleLoginSuccess);
    connect(m_mixModeSelectView, &MixModeSelectView::loginOut, this, &MainView::handleLoginOut);
}

MainView::~MainView()
{
    delete ui;
}

void MainView::handleLoginSuccess(const QString &userName)
{
    m_mainStack->setCurrentIndex(1);
}

void MainView::handleLoginOut()
{
    if(m_loginView == nullptr)
    {
        return;
    }
    m_loginView->reset();
    m_mainStack->setCurrentIndex(0);
}
