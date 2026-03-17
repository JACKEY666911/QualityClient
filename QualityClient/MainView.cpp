#include "MainView.h"
#include "services/httpclient.h"
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
    m_qualityControlView = new QualityControlView(this);
    m_mainStack->addWidget(m_loginView);
    m_mainStack->addWidget(m_mixModeSelectView);
    m_mainStack->addWidget(m_qualityControlView);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_mainStack);
    setLayout(layout);
    connect(m_loginView, &LoginView::loginSuccess, this, &MainView::handleLoginSuccess);
    connect(m_mixModeSelectView, &MixModeSelectView::loginOut, this, &MainView::handleLoginOut);
    connect(m_mixModeSelectView, &MixModeSelectView::enterQualityControl, this, &MainView::handleEnterQualityControl);
    connect(m_qualityControlView, &QualityControlView::requestModeSwitch, this, &MainView::handleSwitchModeRequest);
    m_qualityControlView->setMainImage(QPixmap(":/Images/yisuoTest1.png"));
    m_qualityControlView->setAuxImage(QPixmap(":/Images/yisuoTest2.png"));
}

MainView::~MainView()
{
    delete ui;
}

void MainView::handleLoginSuccess(const QString &userName)
{
    m_mainStack->setCurrentIndex(1);
    if (m_qualityControlView) {
        m_qualityControlView->setUserName(userName);
    }
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

void MainView::handleEnterQualityControl()
{
    m_mainStack->setCurrentIndex(2);
}

void MainView::handleSwitchModeRequest()
{
    m_mainStack->setCurrentIndex(1);
}
