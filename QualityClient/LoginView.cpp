#include "LoginView.h"
#include "ui_LoginView.h"

#include <QLabel>

LoginView::LoginView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginView)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowType::FramelessWindowHint);
    GradientLabel *titleLabel = ui->titleText;

    // 2. 设置渐变颜色（匹配WPF效果：白色→#00E1FF）
    titleLabel->setStartColor(Qt::white);
    titleLabel->setEndColor(QColor("#00E1FF"));

    // 3. 设置渐变方向（垂直渐变，和WPF一致）
    titleLabel->setGradientDirection(GradientDirection::Vertical);
}

LoginView::~LoginView()
{
    delete ui;
}

void LoginView::reset()
{
    ui->userNameInput->clear();
    ui->passwordInput->clear();
    ui->showPassBtn->setChecked(false);
    ui->userNameInput->setFocus();
}

void LoginView::on_showPassBtn_toggled(bool checked)
{
    ui->passwordInput->setEchoMode(checked? QLineEdit::Normal:QLineEdit::Password);
    ui->showPassBtn->setIcon(checked ? QIcon(":/Images/hide_pass.png"):QIcon(":/Images/show_pass.png"));

}


void LoginView::on_loginButton_clicked()
{
    emit loginSuccess(ui->userNameInput->text());
}

