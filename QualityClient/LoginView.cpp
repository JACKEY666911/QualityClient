#include "LoginView.h"
#include "Models/ApiRequests.h"
#include "ui_LoginView.h"

#include <QFutureWatcher>
#include <QLabel>
#include <QMessageBox>

#include <services/apiservice.h>
#include <services/httpclientasync.h>

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

QString LoginView::encryptPassword(const QString &password)
{
    QString trimmedPass = password.trimmed();

    QByteArray buffer = trimmedPass.toLatin1();

    QByteArray hash = QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
    return QString(hash.toHex());
}

void LoginView::on_showPassBtn_toggled(bool checked)
{
    ui->passwordInput->setEchoMode(checked? QLineEdit::Normal:QLineEdit::Password);
    ui->showPassBtn->setIcon(checked ? QIcon(":/Images/hide_pass.png"):QIcon(":/Images/show_pass.png"));

}


void LoginView::on_loginButton_clicked()
{
    // 1. 获取输入并去空格
    QString username = ui->userNameInput->text().trimmed();
    QString password = ui->passwordInput->text();

    // 2. 判空检查
    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名");
        ui->userNameInput->setFocus(); // 自动聚焦到用户名输入框
        return;
    }
    if (password.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入密码");
        ui->passwordInput->setFocus();
        return;
    }
    QString hashedPassword = encryptPassword(password);
    LoginRequest req;
    req.username = username;
    req.password = hashedPassword;
    req.deviceCode = "PC-CLIENT-001";
    req.deviceType = 2000;
    auto future = ApiService::instance().login(req);
    future.then(this, [this, username](const HttpClientAsync::HttpResponse &res){
              ui->loginButton->setEnabled(true);
              if (res.ok) {
                  // 解析登录返回的 Token
                  UserModel user = UserModel::fromJson(res.dataObject());
                  // 设置全局拦截器 Token
                  ApiService::instance().setAuthToken(user.tokenHead, user.token);
                  emit loginSuccess(username);
              } else {
                  // 登录失败：可能是密码错，也可能是服务器连不上
                  QMessageBox::critical(this, "登录失败", res.error);
              }

    }).onFailed(this, [this](const std::exception& e) {
            ui->loginButton->setEnabled(true);
            qDebug() << "网络层异常：" << e.what();
        });
    // 禁用登录按钮，防止重复点击
    ui->loginButton->setEnabled(false);
}

