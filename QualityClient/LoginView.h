#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>

namespace Ui {
class LoginView;
}

class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);
    ~LoginView();

public:
    void reset();

private:
    QString encryptPassword(const QString &password);

signals:
    void loginSuccess(const QString &userName);
    void backToLogin();

private slots:
    void on_showPassBtn_toggled(bool checked);

    void on_loginButton_clicked();

private:
    Ui::LoginView *ui;
};

#endif // LOGINVIEW_H
