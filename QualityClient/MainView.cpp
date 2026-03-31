#include "MainView.h"
#include "logging/logcategories.h"
#include "ui_MainView.h"

#include <QVBoxLayout>

MainView::MainView(QWidget *parent)
    : QWidget(parent), ui(new Ui::MainView), m_mainStack(nullptr),
      m_loginView(nullptr), m_mixModeSelectView(nullptr),
      m_qualityControlView(nullptr) {
  ui->setupUi(this);
  this->setWindowFlags(Qt::FramelessWindowHint);
  this->setWindowState(Qt::WindowFullScreen);
  m_mainStack = new QStackedWidget(this);
  m_loginView = new LoginView(this);
  m_mixModeSelectView = new MixModeSelectView(this);
  m_mainStack->addWidget(m_loginView);
  m_mainStack->addWidget(m_mixModeSelectView);

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_mainStack);
  setLayout(layout);
  connect(m_loginView, &LoginView::loginSuccess, this,
          &MainView::handleLoginSuccess);
  connect(m_mixModeSelectView, &MixModeSelectView::loginOut, this,
          &MainView::handleLoginOut);
  connect(m_mixModeSelectView, &MixModeSelectView::enterQualityControl, this,
          &MainView::handleEnterQualityControl);
  qCInfo(lcMainView) << "MainView initialized";
}

MainView::~MainView() { delete ui; }

void MainView::handleLoginSuccess(const QString &userName) {
  qCInfo(lcMainView) << "Login success, user=" << userName;
  m_currentUserName = userName;
  m_mainStack->setCurrentIndex(1);
  if (m_qualityControlView) {
    m_qualityControlView->setUserName(userName);
  }
}

void MainView::handleLoginOut() {
  if (m_loginView == nullptr) {
    return;
  }
  qCInfo(lcMainView) << "Handle logout";
  destroyQualityControlView();
  m_currentUserName.clear();
  m_loginView->reset();
  m_mainStack->setCurrentIndex(0);
}

void MainView::handleEnterQualityControl() {
  qCInfo(lcMainView) << "Enter QualityControlView";
  createQualityControlView();
  if (m_qualityControlView) {
    m_mainStack->setCurrentWidget(m_qualityControlView);
  }
}

void MainView::handleSwitchModeRequest() {
  qCInfo(lcMainView) << "Switch back to MixModeSelectView";
  destroyQualityControlView();
  m_mainStack->setCurrentIndex(1);
}

void MainView::createQualityControlView() {
  if (m_qualityControlView) {

    return;
  }

  m_qualityControlView = new QualityControlView(this);
  m_mainStack->addWidget(m_qualityControlView);
  connect(m_qualityControlView, &QualityControlView::requestModeSwitch, this,
          &MainView::handleSwitchModeRequest);

  if (!m_currentUserName.isEmpty()) {
    m_qualityControlView->setUserName(m_currentUserName);
  }
}

void MainView::destroyQualityControlView() {
  if (!m_qualityControlView) {
    return;
  }

  m_qualityControlView->clearImageDistributeInfo();
  m_mainStack->removeWidget(m_qualityControlView);
  m_qualityControlView->deleteLater();
  m_qualityControlView = nullptr;
}
