#include "MixModeSelectView.h"
#include "ui_MixModeSelectView.h"

#include <QButtonGroup>
#include <QDebug>

#include <widgets/CheckComBoBox.h>
MixModeSelectView::MixModeSelectView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MixModeSelectView)
{
    ui->setupUi(this);
    QButtonGroup *radioGroup = new QButtonGroup(this);

    radioGroup->addButton(ui->realRadioButton, 1);
    radioGroup->addButton(ui->IntervalRadioButton, 2);
    radioGroup->addButton(ui->normalradioButton, 3);
    connect(radioGroup, qOverload<QAbstractButton*>(&QButtonGroup::buttonClicked),
            this, [=](QAbstractButton *btn) {
        qDebug() << "1111" << btn->text();
    });
    QList<AreaInfo> list;
    list << AreaInfo(1, "北京")
         << AreaInfo(2, "上海")
         << AreaInfo(3, "深圳");

    CheckComBoBox* combo = new CheckComBoBox(this);
    combo->setAreas(list);
    ui->realFrame->layout()->addWidget(combo);
}

MixModeSelectView::~MixModeSelectView()
{
    delete ui;
}

void MixModeSelectView::on_loginoutButton_clicked()
{
    emit loginOut();
}

