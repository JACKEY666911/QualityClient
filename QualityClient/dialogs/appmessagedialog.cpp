#include "dialogs/appmessagedialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AppMessageDialog::AppMessageDialog(const QString &title, const QString &message, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(420);
    setStyleSheet(QStringLiteral(
        "QDialog{background:#ffffff;}"
        "QLabel{color:#333333;font-size:11pt;}"
        "QPushButton{min-width:88px;min-height:34px;background:#117595;color:white;border:none;border-radius:4px;}"
        "QPushButton:hover{background:#1b8aae;}"
    ));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 16);
    layout->setSpacing(16);

    QLabel *msgLabel = new QLabel(message, this);
    msgLabel->setWordWrap(true);
    layout->addWidget(msgLabel);

    QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    QPushButton *okBtn = btnBox->button(QDialogButtonBox::Ok);
    okBtn->setText(QStringLiteral("确定"));
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(btnBox, 0, Qt::AlignRight);
}

void AppMessageDialog::showInfo(QWidget *parent, const QString &title, const QString &message)
{
    AppMessageDialog dialog(title, message, parent);
    dialog.exec();
}
